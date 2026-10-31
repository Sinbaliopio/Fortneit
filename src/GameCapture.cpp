#include "GameCapture.hpp"
#include <dxgi1_2.h>
#include <d3d11.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

GameCapture::~GameCapture() {
    cleanup();
}

bool GameCapture::initialize(int fov_width, int fov_height, int offset_x, int offset_y) {
    std::lock_guard<std::mutex> lock(reinit_mutex_);

    fov_width_ = fov_width;
    fov_height_ = fov_height;
    offset_x_ = offset_x;
    offset_y_ = offset_y;

    cleanup();

    if (!create_device()) {
        std::cerr << "[GameCapture] Failed to create D3D11 device\n";
        return false;
    }

    if (!create_duplication()) {
        std::cerr << "[GameCapture] Failed to create DXGI duplication\n";
        cleanup();
        return false;
    }

    if (!create_staging_texture()) {
        std::cerr << "[GameCapture] Failed to create staging texture\n";
        cleanup();
        return false;
    }

    if (!update_output_geometry()) {
        std::cerr << "[GameCapture] Failed to update output geometry\n";
        cleanup();
        return false;
    }

    initialized_.store(true);
    reinit_requested_.store(false);
    frame_counter_ = 0;

    std::cout << "[GameCapture] Initialized: " << capture_width_ << "x" << capture_height_
              << " at (" << capture_left_ << "," << capture_top_ << ")\n";
    return true;
}

bool GameCapture::create_device() {
    D3D_FEATURE_LEVEL feature_levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D11CreateDevice(
        nullptr,                    // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,   // Hardware acceleration
        nullptr,                    // No software rasterizer
        flags,                      // Flags
        feature_levels,             // Feature levels
        ARRAYSIZE(feature_levels),  // Number of feature levels
        D3D11_SDK_VERSION,          // SDK version
        &device_,                   // Device out
        nullptr,                    // Feature level out
        &context_                   // Context out
    );

    if (FAILED(hr)) {
        std::cerr << "[GameCapture] D3D11CreateDevice failed: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    return true;
}

bool GameCapture::create_duplication() {
    // Get DXGI device from D3D11 device
    ComPtr<IDXGIDevice> dxgi_device;
    HRESULT hr = device_.As(&dxgi_device);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] Failed to get DXGI device: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Get DXGI adapter
    ComPtr<IDXGIAdapter> adapter;
    hr = dxgi_device->GetAdapter(&adapter);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] Failed to get DXGI adapter: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Get primary output (monitor)
    ComPtr<IDXGIOutput> output;
    hr = adapter->EnumOutputs(0, &output);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] Failed to get primary output: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Get output1 interface for Desktop Duplication
    ComPtr<IDXGIOutput1> output1;
    hr = output.As(&output1);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] Failed to get IDXGIOutput1: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Create duplication
    hr = output1->DuplicateOutput(device_.Get(), &duplication_);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] DuplicateOutput failed: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Get duplication description
    hr = duplication_->GetDesc(&duplication_desc_);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] GetDesc failed: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    // Get desktop bounds
    desktop_bounds_ = duplication_desc_.DesktopBounds;

    return true;
}

bool GameCapture::create_staging_texture() {
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = capture_width_;
    desc.Height = capture_height_;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_STAGING;
    desc.BindFlags = 0;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    desc.MiscFlags = 0;

    HRESULT hr = device_->CreateTexture2D(&desc, nullptr, &staging_texture_);
    if (FAILED(hr)) {
        std::cerr << "[GameCapture] CreateTexture2D (staging) failed: 0x" << std::hex << hr << std::dec << "\n";
        return false;
    }

    return true;
}

bool GameCapture::update_output_geometry() {
    // Get screen dimensions from desktop bounds
    screen_width_ = desktop_bounds_.right - desktop_bounds_.left;
    screen_height_ = desktop_bounds_.bottom - desktop_bounds_.top;

    // Calculate capture region centered on screen with FOV and offset
    int center_x = screen_width_ / 2;
    int center_y = screen_height_ / 2;

    capture_width_ = fov_width_;
    capture_height_ = fov_height_;

    capture_left_ = center_x - capture_width_ / 2 + offset_x_;
    capture_top_ = center_y - capture_height_ / 2 + offset_y_;

    // Clamp to screen bounds
    capture_left_ = std::max(0, std::min(capture_left_, screen_width_ - capture_width_));
    capture_top_ = std::max(0, std::min(capture_top_, screen_height_ - capture_height_));

    // Recreate staging texture if dimensions changed
    if (staging_texture_) {
        D3D11_TEXTURE2D_DESC desc;
        staging_texture_->GetDesc(&desc);
        if (desc.Width != static_cast<UINT>(capture_width_) ||
            desc.Height != static_cast<UINT>(capture_height_)) {
            staging_texture_.Reset();
            if (!create_staging_texture()) {
                return false;
            }
        }
    }

    return true;
}

std::optional<CaptureFrame> GameCapture::get_frame() {
    if (!initialized_.load() || reinit_requested_.load()) {
        return std::nullopt;
    }

    // Acquire next frame
    ComPtr<IDXGIResource> desktop_resource;
    DXGI_OUTDUPL_FRAME_INFO frame_info = {};

    HRESULT hr = duplication_->AcquireNextFrame(
        5000,  // 5ms timeout
        &frame_info,
        &desktop_resource
    );

    if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
        return std::nullopt;  // No new frame
    }

    if (hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_DEVICE_REMOVED ||
        hr == DXGI_ERROR_DEVICE_RESET || hr == DXGI_ERROR_SESSION_DISCONNECTED) {
        // Session lost, request reinitialization
        reinit_requested_.store(true);
        initialized_.store(false);
        return std::nullopt;
    }

    if (FAILED(hr)) {
        std::cerr << "[GameCapture] AcquireNextFrame failed: 0x" << std::hex << hr << std::dec << "\n";
        return std::nullopt;
    }

    // Cast to texture
    ComPtr<ID3D11Texture2D> acquired_texture;
    hr = desktop_resource.As(&acquired_texture);
    if (FAILED(hr)) {
        duplication_->ReleaseFrame();
        return std::nullopt;
    }

    // Copy the FOV region to staging texture
    D3D11_BOX src_box = {};
    src_box.left = capture_left_;
    src_box.top = capture_top_;
    src_box.front = 0;
    src_box.right = capture_left_ + capture_width_;
    src_box.bottom = capture_top_ + capture_height_;
    src_box.back = 1;

    context_->CopySubresourceRegion(
        staging_texture_.Get(), 0, 0, 0, 0,
        acquired_texture.Get(), 0, &src_box
    );

    // Map staging texture for CPU read
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    hr = context_->Map(staging_texture_.Get(), 0, D3D11_MAP_READ, 0, &mapped);
    if (SUCCEEDED(hr)) {
        CaptureFrame frame;
        frame.width = capture_width_;
        frame.height = capture_height_;
        frame.stride = static_cast<int>(mapped.RowPitch);
        frame.frame_number = frame_counter_++;
        frame.data.resize(static_cast<size_t>(mapped.RowPitch) * capture_height_);

        // Copy row by row (RowPitch may be wider than width * 4)
        const uint8_t* src = static_cast<const uint8_t*>(mapped.pData);
        uint8_t* dst = frame.data.data();
        int row_bytes = capture_width_ * 4;  // BGRA = 4 bytes

        for (int y = 0; y < capture_height_; ++y) {
            memcpy(dst + y * row_bytes, src + y * mapped.RowPitch, row_bytes);
        }

        context_->Unmap(staging_texture_.Get(), 0);
        duplication_->ReleaseFrame();

        return frame;
    }

    context_->Unmap(staging_texture_.Get(), 0);
    duplication_->ReleaseFrame();
    return std::nullopt;
}

void GameCapture::request_reinit() {
    reinit_requested_.store(true);
}

void GameCapture::cleanup() {
    duplication_.Reset();
    staging_texture_.Reset();
    context_.Reset();
    device_.Reset();
    initialized_.store(false);
}

bool GameCapture::handle_dxgi_error(HRESULT hr) {
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET ||
        hr == DXGI_ERROR_ACCESS_LOST || hr == DXGI_ERROR_SESSION_DISCONNECTED) {
        reinit_requested_.store(true);
        initialized_.store(false);
        return true;
    }
    return false;
}