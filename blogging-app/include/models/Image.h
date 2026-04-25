#pragma once

#include <string>
#include <chrono>

enum class ImageStatus {
    PENDING = 0,      // Waiting for approval
    APPROVED = 1,     // Admin approved for public
    REJECTED = 2,     // Admin rejected
    NSFW_FLAGGED = 3  // NSFW content detected
};

std::string imageStatusToString(ImageStatus status);
ImageStatus imageStatusFromString(const std::string& status);

class Image {
public:
    std::string id;
    std::string album_id;
    std::string url;
    std::string alt_text;
    std::string caption;
    int display_order;
    ImageStatus status;
    bool nsfw_flagged;
    std::string nsfw_reason;
    std::string created_at;
    std::string updated_at;

    Image() : display_order(0), status(ImageStatus::PENDING), nsfw_flagged(false) {}

    // Convert Image to JSON string
    std::string toJson() const;

    // Validate image data
    bool validate() const;
};
