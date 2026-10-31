#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

struct BoundingBox {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    int pixel_count = 0;
    int top_pixel_x = 0;  // Highest pixel X (for head tracking)
    int top_pixel_y = 0;  // Highest pixel Y (for head tracking)

    int center_x() const { return x + width / 2; }
    int center_y() const { return y + height / 2; }

    bool contains(const BoundingBox& other) const {
        return other.x >= x && other.y >= y &&
               other.x + other.width <= x + width &&
               other.y + other.height <= y + height;
    }
    bool intersects(const BoundingBox& other) const {
        return !(other.x > x + width || other.x + other.width < x ||
                 other.y > y + height || other.y + other.height < y);
    }
};

struct DetectionResult {
    std::vector<BoundingBox> clusters;
    std::optional<BoundingBox> best_target;  // Head target (highest pixel of closest cluster)
    int screen_center_x = 0;
    int screen_center_y = 0;
};

class ColorDetector {
public:
    // Cluster distance margin in pixels
    static constexpr int CLUSTER_DISTANCE = 35;
    static constexpr int MIN_PIXEL_COUNT = 5;
    static constexpr int MIN_WIDTH = 3;
    static constexpr int MIN_HEIGHT = 3;

    ColorDetector() = default;

    // Main detection function - processes BGRA buffer
    DetectionResult detect(const uint8_t* bgra_data, int width, int height, int stride);

    // Configure target color (purple hues by default)
    void set_color_thresholds(bool use_high_green_thresholds = true);

private:
    // Color threshold parameters
    struct ColorThresholds {
        // Green >= 120 thresholds
        int green_high_min = 120;
        int red_blue_delta_max_high = 8;
        int red_green_diff_min_high = 50;
        int blue_green_diff_min_high = 50;
        int red_min_high = 105;
        int blue_min_high = 105;

        // Green < 120 thresholds
        int red_blue_delta_max_low = 13;
        int red_green_diff_min_low = 60;
        int blue_green_diff_min_low = 60;
        int red_min_low = 90;
        int blue_min_low = 90;

        bool use_high_green = true;
    } thresholds_;

    // Check if a pixel matches the target color
    inline bool is_target_color(uint8_t b, uint8_t g, uint8_t r) const;

    // Flood fill / connected component labeling for clustering
    void find_clusters(const uint8_t* bgra_data, int width, int height, int stride,
                       std::vector<uint8_t>& visited, std::vector<BoundingBox>& clusters);

    // Merge nearby clusters
    void merge_clusters(std::vector<BoundingBox>& clusters);

    // Filter noise clusters
    void filter_clusters(std::vector<BoundingBox>& clusters, int width, int height);

    // Find best target (closest to center, highest pixel)
    std::optional<BoundingBox> find_best_target(const std::vector<BoundingBox>& clusters,
                                                 int center_x, int center_y);
};