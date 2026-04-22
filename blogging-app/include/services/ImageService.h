#pragma once

#include <string>
#include <vector>
#include <mongocxx/client.hpp>
#include "models/Image.h"

class ImageService {
public:
    // Initialize service
    static bool initialize();
    
    // Image file operations
    static std::string uploadImage(const std::string& file_path, const std::string& file_name);
    static bool deleteImage(const std::string& image_path);
    
    // NSFW Detection (placeholder for ML model integration)
    static bool detectNSFW(const std::string& image_path, float& confidence);
    static bool isNSFWContent(const std::string& image_path, float threshold = 0.5f);
    
    // Image validation
    static bool validateImageFile(const std::string& file_path);
    static bool isAllowedFormat(const std::string& file_name);
    
    // Storage configuration
    static void setStorageBasePath(const std::string& path);
    static std::string getStoragePath();
    
private:
    static std::string storage_base_path;
    static std::vector<std::string> allowed_formats;
};
