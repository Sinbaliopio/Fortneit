#pragma once

#include <atomic>
#include <thread>
#include <chrono>
#include <optional>
#include "Config.hpp"
#include "GameCapture.hpp"
#include "ColorDetection.hpp"
#include "AimLogic.hpp"

// Forward declaration for RawAccel sensitivity state
struct SensitivityState;

class BackendLoop {
public:
    BackendLoop() = default;
    ~BackendLoop();

    // Start the backend thread
    void start(Config& config, SensitivityState* sens_state = nullptr);

    // Stop the backend thread
    void stop();

    // Check if running
    bool is_running() const { return running_.load(); }

    // Get current FPS
    float current_fps() const { return fps_.load(); }

    // Get last detection result (for debugging)
    const DetectionResult& last_detection() const { return last_detection_; }

    // Get last aim output
    const AimOutput& last_aim_output() const { return last_aim_output_; }

private:
    // Main backend thread function
    void run(Config& config, SensitivityState* sens_state);

    // Process a single frame
    void process_frame(Config& config, SensitivityState* sens_state);

    // Print telemetry
    void print_telemetry();

    std::thread backend_thread_;
    std::atomic<bool> running_{false};
    std::atomic<bool> should_stop_{false};

    // Components
    GameCapture capture_;
    ColorDetector detector_;
    AimLogic aim_logic_;

    // State
    std::atomic<float> fps_{0.0f};
    std::atomic<int> frames_this_second_{0};
    std::chrono::steady_clock::time_point last_fps_update_;

    DetectionResult last_detection_;
    AimOutput last_aim_output_;

    // Mouse delta tracking (in real implementation, this would come from driver/input)
    int last_mouse_delta_x_ = 0;
    int last_mouse_delta_y_ = 0;

    // Frame timing
    std::chrono::steady_clock::time_point frame_start_;
};