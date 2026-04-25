#pragma once

#include <string>
#include <vector>

#include "models/Image.h"

enum class AlbumStatus {
    DRAFT,
    SUBMITTED,
    APPROVED,
    PUBLISHED,
    ARCHIVED
};

std::string albumStatusToString(AlbumStatus status);
AlbumStatus albumStatusFromString(const std::string& status);

class Album {
public:
    std::string id;
    std::string title;
    std::string description;
    std::string uploader_name;
    std::string created_by{"admin"};
    std::string admin_token;
    AlbumStatus status{AlbumStatus::DRAFT};
    std::vector<Image> images;
    int image_count{0};
    std::string created_at;
    std::string updated_at;
    std::string published_at;
    bool token_used{false};
    int public_image_count{0};

    std::string toJson(bool include_all_images = false) const;
    bool canAddImage() const { return image_count < 40; }
    int getRemainingSlots() const { return 40 - image_count; }
    bool validate() const;
};
