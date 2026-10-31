#include "ColorDetection.hpp"
#include <queue>
#include <cmath>
#include <limits>
#include <algorithm>

DetectionResult ColorDetector::detect(const uint8_t* bgra_data, int width, int height, int stride) {
    DetectionResult result;
    result.screen_center_x = width / 2;
    result.screen_center_y = height / 2;

    if (!bgra_data || width <= 0 || height <= 0) {
        return result;
    }

    // Visited buffer for flood fill
    std::vector<uint8_t> visited(static_cast<size_t>(width) * height, 0);

    // Find all clusters
    find_clusters(bgra_data, width, height, stride, visited, result.clusters);

    // Merge nearby clusters
    merge_clusters(result.clusters);

    // Filter noise
    filter_clusters(result.clusters, width, height);

    // Find best target (head)
    result.best_target = find_best_target(result.clusters, result.screen_center_x, result.screen_center_y);

    return result;
}

void ColorDetector::set_color_thresholds(bool use_high_green_thresholds) {
    thresholds_.use_high_green = use_high_green_thresholds;
}

inline bool ColorDetector::is_target_color(uint8_t b, uint8_t g, uint8_t r) const {
    if (thresholds_.use_high_green) {
        if (g >= thresholds_.green_high_min) {
            int red_blue_delta = std::abs(static_cast<int>(r) - static_cast<int>(b));
            int red_green_diff = static_cast<int>(r) - static_cast<int>(g);
            int blue_green_diff = static_cast<int>(b) - static_cast<int>(g);

            return (red_blue_delta <= thresholds_.red_blue_delta_max_high &&
                    red_green_diff >= thresholds_.red_green_diff_min_high &&
                    blue_green_diff >= thresholds_.blue_green_diff_min_high &&
                    r >= thresholds_.red_min_high &&
                    b >= thresholds_.blue_min_high);
        }
    }

    // Green < 120 case (or if not using high green thresholds)
    if (g < thresholds_.green_high_min || !thresholds_.use_high_green) {
        int red_blue_delta = std::abs(static_cast<int>(r) - static_cast<int>(b));
        int red_green_diff = static_cast<int>(r) - static_cast<int>(g);
        int blue_green_diff = static_cast<int>(b) - static_cast<int>(g);

        return (red_blue_delta <= thresholds_.red_blue_delta_max_low &&
                red_green_diff >= thresholds_.red_green_diff_min_low &&
                blue_green_diff >= thresholds_.blue_green_diff_min_low &&
                r >= thresholds_.red_min_low &&
                b >= thresholds_.blue_min_low);
    }

    return false;
}

void ColorDetector::find_clusters(const uint8_t* bgra_data, int width, int height, int stride,
                                  std::vector<uint8_t>& visited, std::vector<BoundingBox>& clusters) {
    const int row_bytes = width * 4;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t idx = static_cast<size_t>(y) * width + x;
            if (visited[idx]) continue;

            // Get pixel color (BGRA format)
            const uint8_t* pixel = bgra_data + y * stride + x * 4;
            uint8_t b = pixel[0];
            uint8_t g = pixel[1];
            uint8_t r = pixel[2];
            // pixel[3] is alpha, ignored

            if (!is_target_color(b, g, r)) {
                visited[idx] = 1;
                continue;
            }

            // Found a target pixel - start flood fill (BFS)
            BoundingBox box;
            box.x = x;
            box.y = y;
            box.width = 1;
            box.height = 1;
            box.pixel_count = 0;
            box.top_pixel_x = x;
            box.top_pixel_y = y;

            std::queue<std::pair<int, int>> q;
            q.emplace(x, y);
            visited[idx] = 1;

            while (!q.empty()) {
                auto [cx, cy] = q.front();
                q.pop();

                box.pixel_count++;

                // Update bounding box
                box.x = std::min(box.x, cx);
                box.y = std::min(box.y, cy);
                box.width = std::max(box.width, cx - box.x + 1);
                box.height = std::max(box.height, cy - box.y + 1);

                // Track highest pixel (minimum y)
                if (cy < box.top_pixel_y || (cy == box.top_pixel_y && cx < box.top_pixel_x)) {
                    box.top_pixel_x = cx;
                    box.top_pixel_y = cy;
                }

                // Check 4-connected neighbors
                const int dx[4] = {-1, 1, 0, 0};
                const int dy[4] = {0, 0, -1, 1};

                for (int dir = 0; dir < 4; ++dir) {
                    int nx = cx + dx[dir];
                    int ny = cy + dy[dir];

                    if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;

                    size_t nidx = static_cast<size_t>(ny) * width + nx;
                    if (visited[nidx]) continue;

                    const uint8_t* npixel = bgra_data + ny * stride + nx * 4;
                    uint8_t nb = npixel[0];
                    uint8_t ng = npixel[1];
                    uint8_t nr = npixel[2];

                    if (is_target_color(nb, ng, nr)) {
                        visited[nidx] = 1;
                        q.emplace(nx, ny);
                    } else {
                        visited[nidx] = 1;
                    }
                }
            }

            // Only add cluster if it has enough pixels
            if (box.pixel_count >= MIN_PIXEL_COUNT) {
                clusters.push_back(box);
            }
        }
    }
}

void ColorDetector::merge_clusters(std::vector<BoundingBox>& clusters) {
    bool merged = true;
    int iterations = 0;
    const int max_iterations = 10;

    while (merged && iterations < max_iterations) {
        merged = false;
        iterations++;

        for (size_t i = 0; i < clusters.size(); ++i) {
            for (size_t j = i + 1; j < clusters.size(); ++j) {
                // Check if clusters are close enough to merge
                int dx = std::abs(clusters[i].center_x() - clusters[j].center_x());
                int dy = std::abs(clusters[i].center_y() - clusters[j].center_y());
                double dist = std::sqrt(static_cast<double>(dx * dx + dy * dy));

                if (dist <= CLUSTER_DISTANCE) {
                    // Merge j into i
                    int new_x = std::min(clusters[i].x, clusters[j].x);
                    int new_y = std::min(clusters[i].y, clusters[j].y);
                    int new_right = std::max(clusters[i].x + clusters[i].width, clusters[j].x + clusters[j].width);
                    int new_bottom = std::max(clusters[i].y + clusters[i].height, clusters[j].y + clusters[j].height);

                    // Combine pixel counts
                    clusters[i].pixel_count += clusters[j].pixel_count;

                    // Update top pixel (highest)
                    if (clusters[j].top_pixel_y < clusters[i].top_pixel_y ||
                        (clusters[j].top_pixel_y == clusters[i].top_pixel_y &&
                         clusters[j].top_pixel_x < clusters[i].top_pixel_x)) {
                        clusters[i].top_pixel_x = clusters[j].top_pixel_x;
                        clusters[i].top_pixel_y = clusters[j].top_pixel_y;
                    }

                    clusters[i].x = new_x;
                    clusters[i].y = new_y;
                    clusters[i].width = new_right - new_x;
                    clusters[i].height = new_bottom - new_y;

                    // Remove j
                    clusters.erase(clusters.begin() + j);
                    merged = true;
                    break;
                }
            }
            if (merged) break;
        }
    }
}

void ColorDetector::filter_clusters(std::vector<BoundingBox>& clusters, int width, int height) {
    // Remove clusters that are too small (already filtered in find_clusters, but double-check)
    clusters.erase(
        std::remove_if(clusters.begin(), clusters.end(),
            [this](const BoundingBox& box) {
                return box.pixel_count < MIN_PIXEL_COUNT ||
                       box.width < MIN_WIDTH ||
                       box.height < MIN_HEIGHT;
            }),
        clusters.end()
    );

    // Remove clusters completely enveloped within larger clusters
    for (size_t i = 0; i < clusters.size(); ++i) {
        for (size_t j = 0; j < clusters.size(); ++j) {
            if (i == j) continue;
            if (clusters[j].contains(clusters[i]) && clusters[j].pixel_count > clusters[i].pixel_count) {
                clusters.erase(clusters.begin() + i);
                if (i > 0) --i;
                break;
            }
        }
    }

    // Remove clusters that are mostly outside the screen (safety)
    clusters.erase(
        std::remove_if(clusters.begin(), clusters.end(),
            [width, height](const BoundingBox& box) {
                return box.x + box.width <= 0 || box.x >= width ||
                       box.y + box.height <= 0 || box.y >= height;
            }),
        clusters.end()
    );
}

std::optional<BoundingBox> ColorDetector::find_best_target(const std::vector<BoundingBox>& clusters,
                                                            int center_x, int center_y) {
    if (clusters.empty()) {
        return std::nullopt;
    }

    double best_dist = std::numeric_limits<double>::max();
    const BoundingBox* best_cluster = nullptr;

    for (const auto& cluster : clusters) {
        // Use cluster center for distance calculation
        int cx = cluster.center_x();
        int cy = cluster.center_y();

        double dx = cx - center_x;
        double dy = cy - center_y;
        double dist = dx * dx + dy * dy;  // Squared distance for comparison

        if (dist < best_dist) {
            best_dist = dist;
            best_cluster = &cluster;
        }
    }

    if (best_cluster) {
        // Create result with head position (top pixel of cluster)
        BoundingBox result = *best_cluster;
        result.x = best_cluster->top_pixel_x;
        result.y = best_cluster->top_pixel_y;
        result.width = 1;
        result.height = 1;
        return result;
    }

    return std::nullopt;
}