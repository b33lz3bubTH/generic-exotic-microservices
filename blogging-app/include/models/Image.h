#pragma once
#include <string>

class Image {
public:
    int id;
    int album_id;
    std::string image_url;
    std::string thumbnail_url;
    std::string caption;
    int display_order;
    std::string created_at;
    std::string updated_at;

    Image() : id(-1), album_id(-1), display_order(0) {}
    Image(int albumId, const std::string& imageUrl)
        : id(-1), album_id(albumId), image_url(imageUrl), display_order(0) {}
    
    std::string toJson() const;

    // Convert Image to JSON string
    std::string toJson() const;

    // Validate image data
    bool validate() const;
};
