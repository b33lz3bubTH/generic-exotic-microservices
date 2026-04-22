#pragma once

#include <drogon/drogon.h>
#include <memory>

class AlbumController {
public:
    // Initialize routes
    static void registerRoutes(drogon::HttpAppFramework& app);
    
    // AUTH MIDDLEWARE
    static bool verifyAdminToken(const std::string& token);
    
    // ===== PUBLIC ENDPOINTS (No auth required) =====
    
    // Get all published albums
    static void getPublishedAlbums(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get single album with approved images only
    static void getAlbumById(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const std::string& id);
    
    // Search albums
    static void searchAlbums(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get album statistics
    static void getAlbumStats(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // ===== ADMIN ENDPOINTS (Require X-Admin-Token header) =====
    
    // Create new album (generates admin token)
    static void createAlbum(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Upload images to album (requires valid token)
    static void uploadImagesToAlbum(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get all albums (admin view - all statuses)
    static void getAllAlbums(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get album details with all images (admin)
    static void getAlbumDetailsAdmin(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                    const std::string& id);
    
    // Approve/reject/flag images
    static void approveImage(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    static void rejectImage(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    static void flagImageNSFW(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Publish/archive album
    static void publishAlbum(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const std::string& id);
    
    static void archiveAlbum(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                            const std::string& id);
    
    // Delete album
    static void deleteAlbum(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& id);
};
