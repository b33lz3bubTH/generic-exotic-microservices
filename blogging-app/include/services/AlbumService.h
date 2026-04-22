#pragma once

#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <vector>
#include <memory>
#include "models/Album.h"
#include "models/Image.h"

class AlbumService {
public:
    // Initialize service and connect to database
    static bool initialize(const std::string& connection_string);
    
    // Album CRUD operations
    static std::string createAlbum(const Album& album);
    static Album getAlbumById(const std::string& id);
    static Album getAlbumByToken(const std::string& token);
    static std::vector<Album> getAllAlbums(int page = 1, int limit = 10);
    static std::vector<Album> getAlbumsByStatus(AlbumStatus status, int page = 1, int limit = 10);
    static bool updateAlbum(const std::string& id, const Album& album);
    static bool deleteAlbum(const std::string& id);
    
    // Album status operations
    static bool submitAlbum(const std::string& id);  // Move from DRAFT to SUBMITTED
    static bool approveAlbum(const std::string& id); // SUBMITTED -> APPROVED
    static bool publishAlbum(const std::string& id); // APPROVED -> PUBLISHED
    static bool archiveAlbum(const std::string& id); // Any -> ARCHIVED
    
    // Token operations
    static bool validateAndConsumeToken(const std::string& token);
    static std::string generateAdminToken();
    
    // Image management
    static bool addImagesToAlbum(const std::string& album_id, const std::vector<Image>& images);
    static bool approveImage(const std::string& album_id, const std::string& image_id);
    static bool rejectImage(const std::string& album_id, const std::string& image_id);
    static bool flagImageNSFW(const std::string& album_id, const std::string& image_id, const std::string& reason);
    
    // Utility
    static int getPublishedImageCount(const std::string& album_id);
    static bool checkTokenValidity(const std::string& token);
    
private:
    static std::shared_ptr<mongocxx::client> client;
    static std::string db_name;
    static std::string albums_collection;
};
