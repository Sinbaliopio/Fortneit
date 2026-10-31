#pragma once

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <optional>
#include <string>

using Microsoft::WRL::ComPtr;

struct CaptureFrame {
    std::vector<uint8_t> data;  // BGRA format
    int width = 0;
    int height = 0;
    int stride = 0;  // bytes per row
    uint64_t frame_number = 0;
};

class GameCapture {
public:
    GameCapture() = default;
    ~GameCapture();

    // Non-copyable, movable
    GameCapture(const GameCapture&) = delete;
    GameCapture& operator=(const GameCapture&) = delete;
    GameCapture(GameCapture&&) noexcept = default;
    GameCapture& operator=(GameCapture&&) noexcept = default;

    // Initialize with FOV dimensions (capture region centered on screen)
    bool initialize(int fov_width, int fov_height, int offset_x = 0, int offset_y = 0);

    // Get the next frame, cropped to FOV
    std::optional<CaptureFrame> get_frame();

    // Check if capture is initialized and valid
    bool is_valid() const { return initialized_.load(); }

    // Get current capture dimensions
    int capture_width() const { return capture_width_; }
    int capture_height() const { return capture_height_; }

    // Force reinitialization (e.g., on resolution change)
    void request_reinit();

private:
    // Internal initialization steps
    bool create_device();
    bool create_duplication();
    bool create_staging_texture();
    bool update_output_geometry();

    // Cleanup
    void cleanup();

    // Handle DXGI errors (device lost, session expired, etc.)
    bool handle_dxgi_error(HRESULT hr);

    // COM objects
    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGIOutputDuplication> duplication_;
    ComPtr<ID3D11Texture2D> staging_texture_;

    // Capture parameters
    int capture_width_ = 0;
    int capture_height_ = 0;
    int capture_left_ = 0;
    int capture_top_ = 0;
    int fov_width_ = 0;
    int fov_height_ = 0;
    int offset_x_ = 0;
    int offset_y_ = 0;

    // State
    std::atomic<bool> initialized_{false};
    std::atomic<bool> reinit_requested_{false};
    std::mutex reinit_mutex_;
    uint64_t frame_counter_ = 0;

    // Desktop coordinates
    DXGI_OUTDUPL_DESC duplication_desc_{};
    RECT desktop_bounds_{};

    // Screen dimensions
    int screen_width_ = 1920;
    int screen_height_ = 1080;
};