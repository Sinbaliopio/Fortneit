#pragma once

#include <string>
#include <atomic>
#include <mutex>
#include <optional>
#include <filesystem>

namespace fs = std::filesystem;

struct FovSettings {
    std::atomic<int> max_x{320};
    std::atomic<int> max_y{320};
    std::atomic<int> offset_x{0};
    std::atomic<int> offset_y{0};
};

struct TrackingToggles {
    std::atomic<bool> enable_aim_tracking{false};
    std::atomic<bool> y_axis_freedom{true};
    std::atomic<bool> magnetic_aim_assist{true};
};

struct SensitivityMultipliers {
    std::atomic<float> head_aim_sensitivity{0.35f};
    std::atomic<float> magnetic_acceleration{2.5f};
    std::atomic<float> magnetic_correction{0.15f};
};

struct RadialZones {
    std::atomic<int> magnetic_lock_radius{5};
    std::atomic<int> progressive_slowdown_radius{80};
    std::atomic<int> magnetic_acceleration_radius{200};
    std::atomic<int> magnetic_friction_radius{150};
};

struct CaptureSettings {
    std::atomic<int> capture_method{0}; // 0 = DXGI Desktop Duplication
};

class Config {
public:
    static Config& instance() {
        static Config instance;
        return instance;
    }

    // Non-copyable, non-movable
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    // Configuration access
    FovSettings& fov() { return fov_; }
    const FovSettings& fov() const { return fov_; }

    TrackingToggles& tracking() { return tracking_; }
    const TrackingToggles& tracking() const { return tracking_; }

    SensitivityMultipliers& sensitivity() { return sensitivity_; }
    const SensitivityMultipliers& sensitivity() const { return sensitivity_; }

    RadialZones& radial() { return radial_; }
    const RadialZones& radial() const { return radial_; }

    CaptureSettings& capture() { return capture_; }
    const CaptureSettings& capture() const { return capture_; }

    // File operations
    bool loadFromFile(const fs::path& path = "config.ini");
    bool saveToFile(const fs::path& path = "config.ini") const;
    bool reloadIfChanged();

    // Runtime state (not persisted)
    std::atomic<bool> should_exit{false};
    std::atomic<bool> fov_changed{false};

    // Thread-safe setters for console commands
    void setFovMax(int x, int y);
    void setFovOffset(int x, int y);
    void setTrackingEnabled(bool enabled);
    void setYAxisFreedom(bool enabled);
    void setMagneticAimAssist(bool enabled);
    void setHeadAimSensitivity(float value);
    void setMagneticAcceleration(float value);
    void setMagneticCorrection(float value);
    void setLockRadius(int value);
    void setSlowdownRadius(int value);
    void setAccelerationRadius(int value);
    void setFrictionRadius(int value);
    void setCaptureMethod(int method);

    // Console command parsing
    bool parseCommand(const std::string& command);

    // Help text
    static void printHelp();

private:
    Config() = default;
    ~Config() = default;

    mutable std::mutex file_mutex_;
    fs::file_time_type last_write_time_;

    FovSettings fov_;
    TrackingToggles tracking_;
    SensitivityMultipliers sensitivity_;
    RadialZones radial_;
    CaptureSettings capture_;

    // Helper for INI parsing
    static std::string trim(const std::string& str);
    static std::optional<std::pair<std::string, std::string>> parseLine(const std::string& line);
};