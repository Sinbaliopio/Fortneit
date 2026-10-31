#include "Config.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>

std::string Config::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::optional<std::pair<std::string, std::string>> Config::parseLine(const std::string& line) {
    std::string trimmed = trim(line);
    if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') {
        return std::nullopt;
    }

    size_t eq_pos = trimmed.find('=');
    if (eq_pos == std::string::npos) {
        return std::nullopt;
    }

    std::string key = trim(trimmed.substr(0, eq_pos));
    std::string value = trim(trimmed.substr(eq_pos + 1));

    // Remove inline comments
    size_t comment_pos = value.find(';');
    if (comment_pos != std::string::npos) {
        value = trim(value.substr(0, comment_pos));
    }
    comment_pos = value.find('#');
    if (comment_pos != std::string::npos) {
        value = trim(value.substr(0, comment_pos));
    }

    return std::make_pair(key, value);
}

bool Config::loadFromFile(const fs::path& path) {
    std::lock_guard<std::mutex> lock(file_mutex_);

    std::ifstream file(path);
    if (!file.is_open()) {
        // Create default config if it doesn't exist
        saveToFile(path);
        return true;
    }

    try {
        std::string line;
        while (std::getline(file, line)) {
            auto parsed = parseLine(line);
            if (!parsed) continue;

            const auto& [key, value] = *parsed;

            // FOV Settings
            if (key == "max_x") fov_.max_x.store(std::stoi(value));
            else if (key == "max_y") fov_.max_y.store(std::stoi(value));
            else if (key == "offset_x") fov_.offset_x.store(std::stoi(value));
            else if (key == "offset_y") fov_.offset_y.store(std::stoi(value));

            // Tracking Toggles
            else if (key == "enable_aim_tracking") tracking_.enable_aim_tracking.store(value == "1" || value == "true");
            else if (key == "y_axis_freedom") tracking_.y_axis_freedom.store(value == "1" || value == "true");
            else if (key == "magnetic_aim_assist") tracking_.magnetic_aim_assist.store(value == "1" || value == "true");

            // Sensitivity & Multipliers
            else if (key == "head_aim_sensitivity") sensitivity_.head_aim_sensitivity.store(std::stof(value));
            else if (key == "magnetic_acceleration") sensitivity_.magnetic_acceleration.store(std::stof(value));
            else if (key == "magnetic_correction") sensitivity_.magnetic_correction.store(std::stof(value));

            // Radial Zones
            else if (key == "magnetic_lock_radius") radial_.magnetic_lock_radius.store(std::stoi(value));
            else if (key == "progressive_slowdown_radius") radial_.progressive_slowdown_radius.store(std::stoi(value));
            else if (key == "magnetic_acceleration_radius") radial_.magnetic_acceleration_radius.store(std::stoi(value));
            else if (key == "magnetic_friction_radius") radial_.magnetic_friction_radius.store(std::stoi(value));

            // Capture Settings
            else if (key == "capture_method") capture_.capture_method.store(std::stoi(value));
        }

        last_write_time_ = fs::last_write_time(path);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[Config] Error loading config: " << e.what() << std::endl;
        return false;
    }
}

bool Config::saveToFile(const fs::path& path) const {
    std::lock_guard<std::mutex> lock(file_mutex_);

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "[Config] Failed to open config file for writing: " << path << std::endl;
        return false;
    }

    file << "; Windows Screen Capture & Object Tracking Configuration\n";
    file << "; Generated automatically - modify values as needed\n\n";

    file << "[FOV]\n";
    file << "max_x=" << fov_.max_x.load() << "\n";
    file << "max_y=" << fov_.max_y.load() << "\n";
    file << "offset_x=" << fov_.offset_x.load() << "\n";
    file << "offset_y=" << fov_.offset_y.load() << "\n\n";

    file << "[Tracking]\n";
    file << "enable_aim_tracking=" << (tracking_.enable_aim_tracking.load() ? "1" : "0") << "\n";
    file << "y_axis_freedom=" << (tracking_.y_axis_freedom.load() ? "1" : "0") << "\n";
    file << "magnetic_aim_assist=" << (tracking_.magnetic_aim_assist.load() ? "1" : "0") << "\n\n";

    file << "[Sensitivity]\n";
    file << "head_aim_sensitivity=" << sensitivity_.head_aim_sensitivity.load() << "\n";
    file << "magnetic_acceleration=" << sensitivity_.magnetic_acceleration.load() << "\n";
    file << "magnetic_correction=" << sensitivity_.magnetic_correction.load() << "\n\n";

    file << "[RadialZones]\n";
    file << "magnetic_lock_radius=" << radial_.magnetic_lock_radius.load() << "\n";
    file << "progressive_slowdown_radius=" << radial_.progressive_slowdown_radius.load() << "\n";
    file << "magnetic_acceleration_radius=" << radial_.magnetic_acceleration_radius.load() << "\n";
    file << "magnetic_friction_radius=" << radial_.magnetic_friction_radius.load() << "\n\n";

    file << "[Capture]\n";
    file << "capture_method=" << capture_.capture_method.load() << "\n";

    try {
        last_write_time_ = fs::last_write_time(path);
    } catch (...) {
        // Ignore timestamp errors
    }

    return true;
}

bool Config::reloadIfChanged() {
    fs::path path = "config.ini";
    try {
        auto current_time = fs::last_write_time(path);
        if (current_time != last_write_time_) {
            std::cout << "\n[Config] Configuration file changed, reloading...\n> ";
            std::cout.flush();
            return loadFromFile(path);
        }
    } catch (...) {
        // File might not exist yet
    }
    return false;
}

void Config::setFovMax(int x, int y) {
    fov_.max_x.store(x);
    fov_.max_y.store(y);
    fov_changed.store(true);
}

void Config::setFovOffset(int x, int y) {
    fov_.offset_x.store(x);
    fov_.offset_y.store(y);
}

void Config::setTrackingEnabled(bool enabled) {
    tracking_.enable_aim_tracking.store(enabled);
}

void Config::setYAxisFreedom(bool enabled) {
    tracking_.y_axis_freedom.store(enabled);
}

void Config::setMagneticAimAssist(bool enabled) {
    tracking_.magnetic_aim_assist.store(enabled);
}

void Config::setHeadAimSensitivity(float value) {
    sensitivity_.head_aim_sensitivity.store(value);
}

void Config::setMagneticAcceleration(float value) {
    sensitivity_.magnetic_acceleration.store(value);
}

void Config::setMagneticCorrection(float value) {
    sensitivity_.magnetic_correction.store(value);
}

void Config::setLockRadius(int value) {
    radial_.magnetic_lock_radius.store(value);
}

void Config::setSlowdownRadius(int value) {
    radial_.progressive_slowdown_radius.store(value);
}

void Config::setAccelerationRadius(int value) {
    radial_.magnetic_acceleration_radius.store(value);
}

void Config::setFrictionRadius(int value) {
    radial_.magnetic_friction_radius.store(value);
}

void Config::setCaptureMethod(int method) {
    capture_.capture_method.store(method);
}

bool Config::parseCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty()) return true;

    // Convert to lowercase for case-insensitive matching
    std::transform(cmd.begin(), cmd.end(), cmd.begin(),
                   [](unsigned char c) { return std::tolower(c); });

    if (cmd == "help" || cmd == "h" || cmd == "?") {
        printHelp();
        return true;
    }
    else if (cmd == "exit" || cmd == "quit" || cmd == "q") {
        should_exit.store(true);
        return true;
    }
    else if (cmd == "reload" || cmd == "r") {
        loadFromFile("config.ini");
        std::cout << "[Config] Reloaded from file\n";
        return true;
    }
    else if (cmd == "save" || cmd == "s") {
        saveToFile("config.ini");
        std::cout << "[Config] Saved to file\n";
        return true;
    }
    else if (cmd == "status" || cmd == "st") {
        auto& cfg = *this;
        std::cout << "\n=== Current Configuration ===\n";
        std::cout << "FOV: " << cfg.fov().max_x.load() << "x" << cfg.fov().max_y.load()
                  << " (offset: " << cfg.fov().offset_x.load() << "," << cfg.fov().offset_y.load() << ")\n";
        std::cout << "Tracking: " << (cfg.tracking().enable_aim_tracking.load() ? "ON" : "OFF")
                  << " | Y-Freedom: " << (cfg.tracking().y_axis_freedom.load() ? "ON" : "OFF")
                  << " | Magnetic: " << (cfg.tracking().magnetic_aim_assist.load() ? "ON" : "OFF") << "\n";
        std::cout << "Sensitivity: head=" << cfg.sensitivity().head_aim_sensitivity.load()
                  << " accel=" << cfg.sensitivity().magnetic_acceleration.load()
                  << " corr=" << cfg.sensitivity().magnetic_correction.load() << "\n";
        std::cout << "Radii: lock=" << cfg.radial().magnetic_lock_radius.load()
                  << " slowdown=" << cfg.radial().progressive_slowdown_radius.load()
                  << " accel=" << cfg.radial().magnetic_acceleration_radius.load()
                  << " friction=" << cfg.radial().magnetic_friction_radius.load() << "\n";
        std::cout << "===============================\n";
        return true;
    }
    else if (cmd == "fov") {
        int x, y;
        if (iss >> x >> y) {
            setFovMax(x, y);
            std::cout << "[Config] FOV set to " << x << "x" << y << "\n";
        } else {
            std::cout << "Usage: fov <width> <height>\n";
        }
        return true;
    }
    else if (cmd == "offset") {
        int x, y;
        if (iss >> x >> y) {
            setFovOffset(x, y);
            std::cout << "[Config] FOV offset set to " << x << "," << y << "\n";
        } else {
            std::cout << "Usage: offset <x> <y>\n";
        }
        return true;
    }
    else if (cmd == "tracking") {
        std::string val;
        if (iss >> val) {
            bool enabled = (val == "on" || val == "1" || val == "true");
            setTrackingEnabled(enabled);
            std::cout << "[Config] Tracking " << (enabled ? "enabled" : "disabled") << "\n";
        } else {
            std::cout << "Usage: tracking <on|off>\n";
        }
        return true;
    }
    else if (cmd == "yfreedom" || cmd == "yaxis") {
        std::string val;
        if (iss >> val) {
            bool enabled = (val == "on" || val == "1" || val == "true");
            setYAxisFreedom(enabled);
            std::cout << "[Config] Y-axis freedom " << (enabled ? "enabled" : "disabled") << "\n";
        } else {
            std::cout << "Usage: yfreedom <on|off>\n";
        }
        return true;
    }
    else if (cmd == "magnetic") {
        std::string val;
        if (iss >> val) {
            bool enabled = (val == "on" || val == "1" || val == "true");
            setMagneticAimAssist(enabled);
            std::cout << "[Config] Magnetic aim assist " << (enabled ? "enabled" : "disabled") << "\n";
        } else {
            std::cout << "Usage: magnetic <on|off>\n";
        }
        return true;
    }
    else if (cmd == "headsens" || cmd == "hsens") {
        float val;
        if (iss >> val) {
            setHeadAimSensitivity(val);
            std::cout << "[Config] Head aim sensitivity set to " << val << "\n";
        } else {
            std::cout << "Usage: headsens <value>\n";
        }
        return true;
    }
    else if (cmd == "accel" || cmd == "maccel") {
        float val;
        if (iss >> val) {
            setMagneticAcceleration(val);
            std::cout << "[Config] Magnetic acceleration set to " << val << "\n";
        } else {
            std::cout << "Usage: accel <value>\n";
        }
        return true;
    }
    else if (cmd == "corr" || cmd == "mcorr") {
        float val;
        if (iss >> val) {
            setMagneticCorrection(val);
            std::cout << "[Config] Magnetic correction set to " << val << "\n";
        } else {
            std::cout << "Usage: corr <value>\n";
        }
        return true;
    }
    else if (cmd == "lockradius" || cmd == "lock") {
        int val;
        if (iss >> val) {
            setLockRadius(val);
            std::cout << "[Config] Lock radius set to " << val << "\n";
        } else {
            std::cout << "Usage: lockradius <pixels>\n";
        }
        return true;
    }
    else if (cmd == "slowdown" || cmd == "srad") {
        int val;
        if (iss >> val) {
            setSlowdownRadius(val);
            std::cout << "[Config] Slowdown radius set to " << val << "\n";
        } else {
            std::cout << "Usage: slowdown <pixels>\n";
        }
        return true;
    }
    else if (cmd == "accelradius" || cmd == "arad") {
        int val;
        if (iss >> val) {
            setAccelerationRadius(val);
            std::cout << "[Config] Acceleration radius set to " << val << "\n";
        } else {
            std::cout << "Usage: accelradius <pixels>\n";
        }
        return true;
    }
    else if (cmd == "friction" || cmd == "frad") {
        int val;
        if (iss >> val) {
            setFrictionRadius(val);
            std::cout << "[Config] Friction radius set to " << val << "\n";
        } else {
            std::cout << "Usage: friction <pixels>\n";
        }
        return true;
    }

    std::cout << "Unknown command: " << cmd << ". Type 'help' for available commands.\n";
    return true;
}

void Config::printHelp() {
    std::cout << R"(
=== Available Commands ===
help, h, ?          - Show this help
exit, quit, q       - Exit application
reload, r           - Reload config from config.ini
save, s             - Save current config to config.ini
status, st          - Show current configuration

fov <w> <h>         - Set FOV dimensions (e.g., fov 320 320)
offset <x> <y>      - Set FOV center offset (e.g., offset 0 0)

tracking <on|off>   - Enable/disable aim tracking
yfreedom <on|off>   - Enable/disable Y-axis freedom (recoil compensation)
magnetic <on|off>   - Enable/disable magnetic aim assist

headsens <val>      - Set head aim sensitivity (e.g., headsens 0.35)
accel <val>         - Set magnetic acceleration multiplier (e.g., accel 2.5)
corr <val>          - Set magnetic correction factor (e.g., corr 0.15)

lockradius <px>     - Set inner lock radius (e.g., lockradius 5)
slowdown <px>       - Set progressive slowdown radius (e.g., slowdown 80)
accelradius <px>    - Set acceleration radius (e.g., accelradius 200)
friction <px>       - Set friction radius (e.g., friction 150)

=== Notes ===
- All changes apply immediately without restart
- Config auto-reloads when config.ini changes
- Values are clamped to valid ranges internally
)";
}