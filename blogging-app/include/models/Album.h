#pragma once
#include <string>

class Album {
public:
    int id;
    int user_id;
    std::string title;
    std::string description;
    std::string privacy_level;  // "private", "public", "friends"
    std::string cover_image_url;
    std::string slug;
    std::string created_at;
    std::string updated_at;

    Album() : id(-1), user_id(-1), privacy_level("private") {}
    Album(const std::string& title, int userId)
        : id(-1), user_id(userId), title(title), privacy_level("private") {}
    
    std::string toJson() const;
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
