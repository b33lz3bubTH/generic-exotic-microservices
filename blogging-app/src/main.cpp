#include <drogon/drogon.h>
#include "controllers/BlogController.h"
#include "services/BlogService.h"
#include <iostream>

int main() {
    try {
        // Initialize MongoDB connection
        if (!BlogService::initialize("mongodb://mongo:27017")) {
            std::cerr << "Failed to initialize MongoDB. Make sure MongoDB is running." << std::endl;
            return 1;
        }
        
        std::cout << "Starting Blogging Service..." << std::endl;
        
        auto app = drogon::app();
        
        // Register routes
        BlogController::registerRoutes(*app);
        
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
        
        std::cout << "═══════════════════════════════════════════════" << std::endl;
        std::cout << "        🚀 BLOGGING SERVICE STARTED 🚀" << std::endl;
        std::cout << "═══════════════════════════════════════════════" << std::endl;
        std::cout << std::endl;
        std::cout << "📋 AVAILABLE ENDPOINTS:" << std::endl;
        std::cout << std::endl;
        std::cout << "📝 BLOG OPERATIONS:" << std::endl;
        std::cout << "  POST   /api/blogs                      - Create new blog" << std::endl;
        std::cout << "  GET    /api/blogs                      - Get all blogs (paginated)" << std::endl;
        std::cout << "  GET    /api/blogs/{id}                 - Get blog by ID" << std::endl;
        std::cout << "  GET    /api/blogs/slug/{slug}          - Get blog by slug" << std::endl;
        std::cout << "  PUT    /api/blogs/{id}                 - Update blog" << std::endl;
        std::cout << "  DELETE /api/blogs/{id}                 - Delete blog" << std::endl;
        std::cout << std::endl;
        std::cout << "🔍 SEARCH OPERATIONS:" << std::endl;
        std::cout << "  GET    /api/search?q=query             - Advanced search" << std::endl;
        std::cout << "  GET    /api/search/tag/{tag}           - Search by tag" << std::endl;
        std::cout << "  GET    /api/search/author/{author}     - Search by author" << std::endl;
        std::cout << "  GET    /api/search/category/{category} - Search by category" << std::endl;
        std::cout << std::endl;
        std::cout << "📊 STATS:" << std::endl;
        std::cout << "  GET    /api/stats                      - Get blog statistics" << std::endl;
        std::cout << "  GET    /api/tags/popular               - Get popular tags" << std::endl;
        std::cout << std::endl;
        std::cout << "🔐 PUBLISH MANAGEMENT:" << std::endl;
        std::cout << "  POST   /api/blogs/{id}/publish         - Publish blog" << std::endl;
        std::cout << "  POST   /api/blogs/{id}/unpublish       - Unpublish blog" << std::endl;
        std::cout << std::endl;
        std::cout << "📱 SERVER:" << std::endl;
        std::cout << "  Address: 0.0.0.0:8080" << std::endl;
        std::cout << "  Database: MongoDB (mongo:27017)" << std::endl;
        std::cout << std::endl;
        std::cout << "═══════════════════════════════════════════════" << std::endl;
        std::cout << std::endl;
        
        app->run();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
