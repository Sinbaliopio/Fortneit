#include "AimLogic.hpp"
#include <algorithm>
#include <cmath>

AimOutput AimLogic::compute(const DetectionResult& detection,
                            int mouse_delta_x, int mouse_delta_y,
                            const Config& config) {
    AimOutput output;

    // Crosshair is at screen center (with FOV offset)
    output.crosshair_x = detection.screen_center_x + config.fov().offset_x.load();
    output.crosshair_y = detection.screen_center_y + config.fov().offset_y.load();

    // Check if tracking is enabled and we have a target
    if (!config.tracking().enable_aim_tracking.load() ||
        !config.tracking().magnetic_aim_assist.load() ||
        !detection.best_target.has_value()) {
        output.multiplier = 1.0f;
        output.bypassY = false;
        output.target_found = false;
        last_output_ = output;
        return output;
    }

    const auto& target = detection.best_target.value();
    output.target_x = target.x;
    output.target_y = target.y;
    output.target_found = true;

    // Calculate distance from crosshair to target
    float distance = calculate_distance(output.crosshair_x, output.crosshair_y,
                                        target.x, target.y,
                                        config.fov().offset_x.load(),
                                        config.fov().offset_y.load());
    output.target_distance = distance;

    // Calculate direction vectors
    int target_dx = target.x - output.crosshair_x;
    int target_dy = target.y - output.crosshair_y;

    // Calculate dot product to determine if moving towards/away
    float dot_product = calculate_dot_product(mouse_delta_x, mouse_delta_y,
                                              target_dx, target_dy);

    // Apply concentric radii logic
    float multiplier = apply_radial_logic(distance, dot_product, config);

    // Apply Y-axis freedom logic
    bool bypassY = apply_y_freedom(distance, dot_product, config);

    // Apply head aim sensitivity in lock radius
    int lock_radius = config.radial().magnetic_lock_radius.load();
    if (distance < lock_radius) {
        multiplier = config.sensitivity().head_aim_sensitivity.load();
    }

    // Clamp multiplier to reasonable range
    multiplier = std::clamp(multiplier, 0.0f, 10.0f);

    // Smooth transition (optional: simple lerp with previous frame)
    const float smoothing = 0.15f;
    multiplier = previous_multiplier_ * (1.0f - smoothing) + multiplier * smoothing;

    output.multiplier = multiplier;
    output.bypassY = bypassY;

    last_output_ = output;
    previous_multiplier_ = multiplier;

    return output;
}

void AimLogic::update(const AimOutput& output) {
    last_output_ = output;
    previous_multiplier_ = output.multiplier;
}

float AimLogic::calculate_distance(int crosshair_x, int crosshair_y,
                                   int target_x, int target_y,
                                   int offset_x, int offset_y) const {
    // Crosshair position already includes offset
    int dx = target_x - crosshair_x;
    int dy = target_y - crosshair_y;
    return std::sqrt(static_cast<float>(dx * dx + dy * dy));
}

float AimLogic::calculate_dot_product(int mouse_dx, int mouse_dy,
                                      int target_dx, int target_dy) const {
    // Normalize mouse delta
    float mouse_len = std::sqrt(static_cast<float>(mouse_dx * mouse_dx + mouse_dy * mouse_dy));
    if (mouse_len < 0.1f) return 0.0f;  // No mouse movement

    float mouse_nx = mouse_dx / mouse_len;
    float mouse_ny = mouse_dy / mouse_len;

    // Normalize target direction
    float target_len = std::sqrt(static_cast<float>(target_dx * target_dx + target_dy * target_dy));
    if (target_len < 0.1f) return 1.0f;  // Already on target

    float target_nx = target_dx / target_len;
    float target_ny = target_dy / target_len;

    // Dot product: 1 = same direction, -1 = opposite, 0 = perpendicular
    return mouse_nx * target_nx + mouse_ny * target_ny;
}

float AimLogic::apply_radial_logic(float distance, float dot_product,
                                   const Config& config) const {
    int lock_radius = config.radial().magnetic_lock_radius.load();
    int slowdown_radius = config.radial().progressive_slowdown_radius.load();
    int accel_radius = config.radial().magnetic_acceleration_radius.load();
    int friction_radius = config.radial().magnetic_friction_radius.load();

    float head_sens = config.sensitivity().head_aim_sensitivity.load();
    float max_accel = config.sensitivity().magnetic_acceleration.load();
    float correction = config.sensitivity().magnetic_correction.load();

    // Ring 1: Inner Lock (handled in compute(), but included for completeness)
    if (distance < lock_radius) {
        return head_sens;
    }

    // Ring 2: Middle Slowdown (progressive slowdown)
    if (distance < slowdown_radius) {
        // Power curve: multiplier decreases as distance approaches lock_radius
        // At slowdown_radius: multiplier = 1.0
        // At lock_radius: multiplier = head_sens
        float t = (distance - lock_radius) / static_cast<float>(slowdown_radius - lock_radius);
        t = std::clamp(t, 0.0f, 1.0f);

        // Power curve for smooth transition
        float curve = std::pow(t, 0.5f);  // Square root curve
        return head_sens + (1.0f - head_sens) * curve;
    }

    // Ring 3: Outer Acceleration (pull towards target)
    if (distance < accel_radius) {
        if (dot_product > 0.0f) {  // Moving TOWARDS target
            // Scale from 1.0 at slowdown_radius to max_accel at accel_radius
            float t = (distance - slowdown_radius) / static_cast<float>(accel_radius - slowdown_radius);
            t = std::clamp(t, 0.0f, 1.0f);
            return 1.0f + (max_accel - 1.0f) * t * dot_product;
        }
        return 1.0f;
    }

    // Ring 4: Friction (resistance when moving AWAY)
    if (distance < friction_radius && dot_product < 0.0f) {
        // Exponentially reduce multiplier based on inverse dot product
        // dot_product is negative when moving away, so -dot_product is positive
        float away_factor = -dot_product;  // 0 to 1
        float t = 1.0f - (distance - accel_radius) / static_cast<float>(friction_radius - accel_radius);
        t = std::clamp(t, 0.0f, 1.0f);

        // Apply correction factor
        float reduction = 1.0f - correction * away_factor * t;
        return std::max(reduction, 0.1f);  // Don't go below 0.1x
    }

    // Outside all rings: no modification
    return 1.0f;
}

bool AimLogic::apply_y_freedom(float distance, float dot_product,
                               const Config& config) const {
    if (!config.tracking().y_axis_freedom.load()) {
        return false;
    }

    int lock_radius = config.radial().magnetic_lock_radius.load();

    // Only apply when horizontally locked (within lock radius on X axis)
    // and player is pulling down (positive Y delta = down in screen coords)
    // Note: We check this in compute() with actual mouse delta, here we just
    // return whether the condition COULD be met based on distance

    // The actual bypassY decision needs mouse delta, so we return true
    // if we're in the lock radius horizontally, and the calling code
    // will check the mouse Y direction
    return distance < lock_radius;
}