#include "BackendLoop.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

// Mock SensitivityState for compilation (replace with actual RawAccel implementation)
struct SensitivityState {
    void updateDynamic(float multiplier, bool bypassY) {
        // In real implementation, this sends IOCTL to RawAccel driver
        // For now, just store the values
        last_multiplier = multiplier;
        last_bypassY = bypassY;
    }
    float last_multiplier = 1.0f;
    bool last_bypassY = false;
};

BackendLoop::~BackendLoop() {
    stop();
}

void BackendLoop::start(Config& config, SensitivityState* sens_state) {
    if (running_.load()) return;

    should_stop_.store(false);
    running_.store(true);
    last_fps_update_ = std::chrono::steady_clock::now();

    backend_thread_ = std::thread(&BackendLoop::run, this, std::ref(config), sens_state);
    // Detach so it runs independently
    backend_thread_.detach();
}

void BackendLoop::stop() {
    should_stop_.store(true);
    running_.store(false);
}

void BackendLoop::run(Config& config, SensitivityState* sens_state) {
    std::cout << "[Backend] Thread started\n";

    // Initialize capture with current FOV
    if (!capture_.initialize(config.fov().max_x.load(), config.fov().max_y.load(),
                             config.fov().offset_x.load(), config.fov().offset_y.load())) {
        std::cerr << "[Backend] Failed to initialize capture\n";
        running_.store(false);
        return;
    }

    while (!should_stop_.load()) {
        frame_start_ = std::chrono::steady_clock::now();

        // Step 1: PAUSE CHECK
        if (!config.tracking().enable_aim_tracking.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }

        // Step 2: FOV CHANGE CHECK
        if (config.fov_changed.load()) {
            std::cout << "[Backend] FOV changed, reinitializing capture...\n";
            capture_.request_reinit();

            // Wait a moment for cleanup
            std::this_thread::sleep_for(std::chrono::milliseconds(50));

            if (!capture_.initialize(config.fov().max_x.load(), config.fov().max_y.load(),
                                     config.fov().offset_x.load(), config.fov().offset_y.load())) {
                std::cerr << "[Backend] Failed to reinitialize capture\n";
            } else {
                config.fov_changed.store(false);
            }
        }

        // Step 3: FRAME ACQUISITION
        auto frame_opt = capture_.get_frame();

        // Step 4: SCANNING
        if (frame_opt.has_value()) {
            const auto& frame = frame_opt.value();
            last_detection_ = detector_.detect(frame.data.data(), frame.width, frame.height, frame.stride);
        }

        // Step 5: AIM LOGIC + DRIVER UPDATE
        AimOutput aim_output = aim_logic_.compute(last_detection_,
                                                   last_mouse_delta_x_, last_mouse_delta_y_,
                                                   config);
        last_aim_output_ = aim_output;

        if (sens_state && aim_output.target_found) {
            sens_state->updateDynamic(aim_output.multiplier, aim_output.bypassY);
        }

        // Step 6: TELEMETRY OUTPUT
        print_telemetry();

        // Frame rate limiting / timing
        auto frame_end = std::chrono::steady_clock::now();
        auto frame_time = std::chrono::duration_cast<std::chrono::microseconds>(frame_end - frame_start_);

        // Update FPS counter
        frames_this_second_++;
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_fps_update_);
        if (elapsed.count() >= 1) {
            fps_.store(static_cast<float>(frames_this_second_.load()));
            frames_this_second_.store(0);
            last_fps_update_ = now;
        }

        // Small sleep to prevent 100% CPU if frames are very fast
        // Target ~144 FPS max (approx 7ms per frame)
        if (frame_time.count() < 7000) {
            std::this_thread::sleep_for(std::chrono::microseconds(7000 - frame_time.count()));
        }
    }

    std::cout << "\n[Backend] Thread stopped\n";
}

void BackendLoop::print_telemetry() {
    // Use carriage return to update in place
    // Format: FPS: XXX | Targets: X | Dist: XXX | Mult: X.XX | BypassY: Y
    std::cout << "\r[Telemetry] "
              << "FPS: " << std::setw(3) << static_cast<int>(fps_.load())
              << " | Targets: " << std::setw(2) << last_detection_.clusters.size()
              << " | Found: " << (last_detection_.best_target.has_value() ? "YES" : "NO ")
              << " | Dist: " << std::setw(6) << std::fixed << std::setprecision(1) << last_aim_output_.target_distance
              << " | Mult: " << std::setw(5) << std::fixed << std::setprecision(2) << last_aim_output_.multiplier
              << " | BypassY: " << (last_aim_output_.bypassY ? "ON" : "OFF")
              << " | Target: (" << last_aim_output_.target_x << "," << last_aim_output_.target_y << ")"
              << std::flush;
}