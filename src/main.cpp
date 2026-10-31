#include "Config.hpp"
#include "BackendLoop.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>

// Mock SensitivityState for standalone compilation
// Replace with actual RawAccel implementation when available
struct SensitivityState {
    void updateDynamic(float multiplier, bool bypassY) {
        last_multiplier = multiplier;
        last_bypassY = bypassY;
    }
    float last_multiplier = 1.0f;
    bool last_bypassY = false;
};

int main() {
    // Set console title
    SetConsoleTitleA("Screen Capture & Object Tracking - Console");

    std::cout << R"(
===============================================================================
  Windows Screen Capture & Computer-Vision Object Tracking
  Console Application - C++20
===============================================================================
)";

    // Get config instance
    Config& config = Config::instance();

    // Load configuration
    if (!config.loadFromFile("config.ini")) {
        std::cerr << "[Main] Failed to load config, using defaults\n";
    }

    // Create sensitivity state (RawAccel integration)
    SensitivityState sens_state;

    // Create and start backend
    BackendLoop backend;
    backend.start(config, &sens_state);

    // Main console input loop
    std::string input;
    std::cout << "\n> ";
    std::cout.flush();

    while (!config.should_exit.load()) {
        // Check for config file changes
        config.reloadIfChanged();

        // Non-blocking input check
        if (std::cin.rdbuf()->in_avail() > 0) {
            std::getline(std::cin, input);

            if (!input.empty()) {
                // Parse command
                config.parseCommand(input);
            }

            if (!config.should_exit.load()) {
                std::cout << "> ";
                std::cout.flush();
            }
        } else {
            // Small sleep to prevent busy waiting
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    // Cleanup
    std::cout << "\n[Main] Shutting down...\n";
    backend.stop();

    // Give backend thread time to stop
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Save config on exit
    config.saveToFile("config.ini");
    std::cout << "[Main] Configuration saved. Goodbye!\n";

    return 0;
}