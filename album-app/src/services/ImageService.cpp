#include "services/ImageService.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring>

namespace fs = std::filesystem;

std::string ImageService::storage_base_path = "./uploads/images";
std::vector<std::string> ImageService::allowed_formats = {"jpg", "jpeg", "png"};

bool ImageService::initialize() {
    try {
        fs::create_directories(storage_base_path);
        std::cout << "✓ Image storage initialized at " << storage_base_path << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "✗ Error initializing image storage: " << e.what() << std::endl;
        return false;
    }
}

std::string ImageService::uploadImage(const std::string& file_path, const std::string& file_name) {
    try {
        if (!validateImageFile(file_path)) {
            std::cerr << "Invalid image file: " << file_path << std::endl;
            return "";
        }
        
        std::string destination = storage_base_path + "/" + file_name;
        fs::copy_file(file_path, destination, fs::copy_options::overwrite_existing);
        
        std::cout << "✓ Image uploaded: " << file_name << std::endl;
        return destination;
    } catch (const std::exception& e) {
        std::cerr << "✗ Error uploading image: " << e.what() << std::endl;
        return "";
    }
}

bool ImageService::deleteImage(const std::string& image_path) {
    try {
        if (fs::exists(image_path)) {
            fs::remove(image_path);
            std::cout << "✓ Image deleted: " << image_path << std::endl;
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "✗ Error deleting image: " << e.what() << std::endl;
    }
    
    return false;
}

bool ImageService::validateImageFile(const std::string& file_path) {
    try {
        if (!fs::exists(file_path)) {
            std::cerr << "File does not exist: " << file_path << std::endl;
            return false;
        }
        
        auto size = fs::file_size(file_path);
        const long MAX_FILE_SIZE = 50 * 1024 * 1024;  // 50MB
        
        if (size > MAX_FILE_SIZE) {
            std::cerr << "File too large: " << size << " bytes (max: " << MAX_FILE_SIZE << ")" << std::endl;
            return false;
        }
        
        if (size == 0) {
            std::cerr << "File is empty" << std::endl;
            return false;
        }
        
        return isAllowedFormat(file_path);
    } catch (const std::exception& e) {
        std::cerr << "Error validating image: " << e.what() << std::endl;
        return false;
    }
}

bool ImageService::isAllowedFormat(const std::string& file_name) {
    // Get file extension
    size_t lastdot = file_name.find_last_of(".");
    if (lastdot == std::string::npos) {
        return false;
    }
    
    std::string ext = file_name.substr(lastdot + 1);
    
    // Convert to lowercase
    for (auto& ch : ext) {
        ch = std::tolower(ch);
    }
    
    for (const auto& allowed : allowed_formats) {
        if (ext == allowed) {
            return true;
        }
    }
    
    return false;
}

bool ImageService::detectNSFW(const std::string& image_path, float& confidence) {
    // Placeholder for NSFW detection integration
    // In production, this would integrate with a ML model or external service
    // For now, return false (no NSFW detected)
    std::cout << "⚠ NSFW detection is a placeholder (not implemented)" << std::endl;
    confidence = 0.0f;
    return false;
}

bool ImageService::isNSFWContent(const std::string& image_path, float threshold) {
    float confidence = 0.0f;
    bool is_nsfw = detectNSFW(image_path, confidence);
    
    std::cout << "NSFW Confidence: " << confidence << std::endl;
    return is_nsfw && confidence > threshold;
}

void ImageService::setStorageBasePath(const std::string& path) {
    storage_base_path = path;
    try {
        fs::create_directories(storage_base_path);
    } catch (const std::exception& e) {
        std::cerr << "Error setting storage path: " << e.what() << std::endl;
    }
}

std::string ImageService::getStoragePath() {
    return storage_base_path;
}
