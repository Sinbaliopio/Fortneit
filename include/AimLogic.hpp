#pragma once

#include <atomic>
#include <cmath>
#include "Config.hpp"
#include "ColorDetection.hpp"

// Forward declaration for RawAccel sensitivity state (provided separately)
struct SensitivityState;

// Aim logic output
struct AimOutput {
    float multiplier = 1.0f;
    bool bypassY = false;
    bool target_found = false;
    float target_distance = 0.0f;
    int target_x = 0;
    int target_y = 0;
    int crosshair_x = 0;
    int crosshair_y = 0;
};

class AimLogic {
public:
    AimLogic() = default;

    // Compute aim multiplier and bypassY based on current state
    AimOutput compute(const DetectionResult& detection,
                      int mouse_delta_x, int mouse_delta_y,
                      const Config& config);

    // Update internal state (call each frame)
    void update(const AimOutput& output);

    // Get last computed output
    const AimOutput& last_output() const { return last_output_; }

private:
    // Calculate distance from crosshair to target
    float calculate_distance(int crosshair_x, int crosshair_y,
                             int target_x, int target_y,
                             int offset_x, int offset_y) const;

    // Calculate dot product to determine if moving towards/away
    float calculate_dot_product(int mouse_dx, int mouse_dy,
                                int target_dx, int target_dy) const;

    // Apply concentric radii logic
    float apply_radial_logic(float distance, float dot_product,
                             const Config& config) const;

    // Apply Y-axis freedom logic
    bool apply_y_freedom(float distance, float dot_product,
                         const Config& config) const;

    AimOutput last_output_{};
    float previous_multiplier_ = 1.0f;
};