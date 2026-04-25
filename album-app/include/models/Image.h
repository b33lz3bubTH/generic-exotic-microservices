#pragma once

#include <string>

enum class ImageStatus {
    PENDING,
    APPROVED,
    REJECTED,
    NSFW_FLAGGED
};

std::string imageStatusToString(ImageStatus status);
ImageStatus imageStatusFromString(const std::string& status);
bool canTransitionImageStatus(ImageStatus from, ImageStatus to);

class Image {
public:
    std::string id;
    std::string album_id;
    std::string url;
    std::string alt_text;
    std::string caption;
    int display_order{0};
    ImageStatus status{ImageStatus::PENDING};
    bool nsfw_flagged{false};
    std::string nsfw_reason;
    std::string created_at;
    std::string updated_at;

    std::string toJson() const;
    bool validate() const;
};
