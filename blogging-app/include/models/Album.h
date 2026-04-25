#pragma once

#include <string>
#include <vector>
#include <chrono>
#include "Image.h"

enum class AlbumStatus {
    DRAFT = 0,        // Initial state, awaiting image submission
    SUBMITTED = 1,    // All images uploaded, awaiting approval
    APPROVED = 2,     // Admin approved
    PUBLISHED = 3,    // Live and visible
    ARCHIVED = 4      // Archived/inactive
};

std::string albumStatusToString(AlbumStatus status);
AlbumStatus albumStatusFromString(const std::string& status);

class Album {
public:
    std::string id;
    std::string title;
    std::string description;
    std::string admin_token;  // One-time use token to upload images
    AlbumStatus status;
    std::vector<Image> images;
    int image_count;
    std::string uploader_name;    // Name of the person who uploaded/created the album
    std::string created_by;       // Admin/system ID
    std::string created_at;
    std::string updated_at;
    std::string published_at;
    bool token_used;              // Token consumption flag (one-time use)
    
    // Metadata
    int public_image_count;       // Count of approved images
    
    Album() : status(AlbumStatus::DRAFT), image_count(0), token_used(false), public_image_count(0) {}

    // Convert Album to JSON string
    std::string toJson(bool include_all_images = false) const;

    // Helper functions
    bool canAddImage() const { return image_count < 40; }
    int getRemainingSlots() const { return 40 - image_count; }
    bool validate() const;
};
