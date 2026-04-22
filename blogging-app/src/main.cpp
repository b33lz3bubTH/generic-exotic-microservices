#include <drogon/drogon.h>
#include "controllers/AlbumController.h"
#include "services/AlbumService.h"
#include "services/ImageService.h"
#include <iostream>

int main() {
    try {
        // Initialize MongoDB connection
        if (!AlbumService::initialize("mongodb://mongo:27017")) {
            std::cerr << "Failed to initialize MongoDB. Make sure MongoDB is running." << std::endl;
            return 1;
        }
        
        // Initialize image storage
        if (!ImageService::initialize()) {
            std::cerr << "Failed to initialize image storage." << std::endl;
            return 1;
        }
        
        std::cout << "Starting Photo Sharing Service..." << std::endl;
        
        auto app = drogon::app();
        
        // Register routes
        AlbumController::registerRoutes(*app);
        
        // Add CORS headers for all responses
        app->registerPostRoutingAdvice(
            [](const drogon::HttpRequestPtr& req, const drogon::AdviceCallback& AdviceCallback) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin", "*");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
                AdviceCallback(resp);
            }
        );
        
        // Handle OPTIONS requests
        app->registerHandler(
            "/api/blogs",
            [](const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
                auto resp = drogon::HttpResponse::newHttpResponse();
                resp->addHeader("Access-Control-Allow-Origin", "*");
                resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
                resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
                callback(resp);
            },
            {drogon::Options}
        );
        
        // Configure server
        app->addListener("0.0.0.0", 8080);
        
        // Thread pool and other settings
        app->setThreadNum(4);
        
        std::cout << "═══════════════════════════════════════════════════════" << std::endl;
        std::cout << "      🖼️  PHOTO SHARING SERVICE STARTED 🖼️" << std::endl;
        std::cout << "═══════════════════════════════════════════════════════" << std::endl;
        std::cout << std::endl;
        std::cout << "📋 PUBLIC ENDPOINTS (No auth required):" << std::endl;
        std::cout << std::endl;
        std::cout << "📸 ALBUM OPERATIONS:" << std::endl;
        std::cout << "  GET    /api/albums                     - Get all published albums" << std::endl;
        std::cout << "  GET    /api/albums/{id}                - Get album with approved images" << std::endl;
        std::cout << "  GET    /api/albums/search?q=query      - Search albums" << std::endl;
        std::cout << "  GET    /api/albums/stats/overview      - Get album statistics" << std::endl;
        std::cout << std::endl;
        std::cout << "🔐 ADMIN ENDPOINTS (Require admin setup):" << std::endl;
        std::cout << std::endl;
        std::cout << "💾 ALBUM MANAGEMENT:" << std::endl;
        std::cout << "  POST   /api/admin/albums               - Create new album (get token)" << std::endl;
        std::cout << "  GET    /api/admin/albums               - Get all albums (admin view)" << std::endl;
        std::cout << "  GET    /api/admin/albums/{id}          - Get album with all images" << std::endl;
        std::cout << "  DELETE /api/admin/albums/{id}          - Delete album" << std::endl;
        std::cout << "  POST   /api/admin/albums/{id}/publish  - Publish album" << std::endl;
        std::cout << "  POST   /api/admin/albums/{id}/archive  - Archive album" << std::endl;
        std::cout << std::endl;
        std::cout << "🖼️  IMAGE MANAGEMENT:" << std::endl;
        std::cout << "  POST   /api/admin/images/upload        - Upload images (with token)" << std::endl;
        std::cout << "  POST   /api/admin/images/{id}/approve  - Approve image" << std::endl;
        std::cout << "  POST   /api/admin/images/{id}/reject   - Reject image" << std::endl;
        std::cout << "  POST   /api/admin/images/{id}/nsfw     - Flag image as NSFW" << std::endl;
        std::cout << std::endl;
        std::cout << "📝 FEATURES:" << std::endl;
        std::cout << "  ✓ Albums max 40 images each" << std::endl;
        std::cout << "  ✓ One-time tokens per album for uploads" << std::endl;
        std::cout << "  ✓ Image alt text, caption, and URL support" << std::endl;
        std::cout << "  ✓ Individual image approval/rejection" << std::endl;
        std::cout << "  ✓ NSFW detection placeholder" << std::endl;
        std::cout << "  ✓ Local storage (Cloudflare R2 ready)" << std::endl;
        std::cout << std::endl;
        std::cout << "📱 SERVER:" << std::endl;
        std::cout << "  Address: 0.0.0.0:8080" << std::endl;
        std::cout << "  Database: MongoDB (mongo:27017)" << std::endl;
        std::cout << "  Storage: ./uploads/images" << std::endl;
        std::cout << std::endl;
        std::cout << "═══════════════════════════════════════════════════════" << std::endl;
        std::cout << std::endl;
        
        app->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
