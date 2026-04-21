#pragma once

#include <drogon/drogon.h>
#include <memory>

class BlogController {
public:
    // Initialize routes
    static void registerRoutes(drogon::HttpAppFramework& app);
    
    // AUTH MIDDLEWARE
    static bool verifyAdminKey(const drogon::HttpRequestPtr& req);
    
    // ===== PUBLIC ENDPOINTS (No auth required) =====
    
    // Submit blog for approval
    static void submitBlogPublic(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get published blogs
    static void getPublishedBlogs(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Get blog by slug
    static void getBlogBySlug(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                             const std::string& slug);
    
    // Public search
    static void searchBlogs(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // ===== ADMIN ENDPOINTS (Require X-API-Key header) =====
    
    // Admin: Create blog  
    static void createBlog(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Admin: Get all blogs (all statuses)
    static void getAllBlogs(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Admin: Get blog by ID
    static void getBlogById(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& id);
    
    // Admin: Update blog
    static void updateBlog(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                          const std::string& id);
    
    // Admin: Delete blog
    static void deleteBlog(const drogon::HttpRequestPtr& req,
                          std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                          const std::string& id);
    
    // Admin: Get submitted blogs awaiting approval
    static void getSubmittedBlogs(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Admin: Get draft blogs
    static void getDraftBlogs(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Admin: Search by tag
    static void searchByTag(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& tag);
    
    // Admin: Search by author
    static void searchByAuthor(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                              const std::string& author);
    
    // Admin: Search by category
    static void searchByCategory(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                const std::string& category);
    
    // Admin: Get stats
    static void getBlogStats(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // Admin: Get popular tags
    static void getPopularTags(const drogon::HttpRequestPtr& req,
                              std::function<void(const drogon::HttpResponsePtr&)>&& callback);
    
    // ===== STATUS TRANSITIONS (Admin only) =====
    
    // Approve submitted blog
    static void approveBlog(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& id);
    
    // Move blog to draft
    static void moveToDraft(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& id);
    
    // Publish blog
    static void publishBlog(const drogon::HttpRequestPtr& req,
                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                           const std::string& id);

private:
    static Json::Value createErrorResponse(const std::string& message);
    static Json::Value createSuccessResponse(const std::string& message, const Json::Value& data);
};
