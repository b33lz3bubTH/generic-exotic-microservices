#pragma once

#include "models/BlogPost.h"
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <string>
#include <vector>
#include <memory>

class BlogService {
private:
    mongocxx::database db;
    static mongocxx::client mongo_client;
    static bool initialized;

public:
    BlogService();
    
    // Initialize MongoDB connection
    static bool initialize(const std::string& uri);
    
    // CRUD Operations
    std::string createBlog(const BlogPost& blog);
    BlogPost getBlogById(const std::string& id);
    BlogPost getBlogBySlug(const std::string& slug);
    std::vector<BlogPost> getAllBlogs(int page = 1, int limit = 10);
    std::vector<BlogPost> updateBlog(const std::string& id, const BlogPost& blog);
    bool deleteBlog(const std::string& id);
    
    // Search Operations
    std::vector<BlogPost> searchByTitle(const std::string& query, int page = 1, int limit = 10);
    std::vector<BlogPost> searchByTag(const std::string& tag, int page = 1, int limit = 10);
    std::vector<BlogPost> searchByAuthor(const std::string& author, int page = 1, int limit = 10);
    std::vector<BlogPost> searchByCategory(const std::string& category, int page = 1, int limit = 10);
    std::vector<BlogPost> advancedSearch(
        const std::string& query = "",
        const std::vector<std::string>& tags = {},
        const std::string& author = "",
        const std::string& category = "",
        int page = 1,
        int limit = 10
    );
    
    // Stats
    long long getTotalBlogCount();
    long long getTotalBlogCountByCategory(const std::string& category);
    std::vector<std::pair<std::string, long long>> getPopularTags();
    std::vector<std::pair<std::string, long long>> getCategories();
    
    // Increment view count
    bool incrementViewCount(const std::string& id);
    
    // Status transitions (Admin only)
    bool approveBlog(const std::string& id);
    bool moveToDraft(const std::string& id);
    bool publishBlog(const std::string& id);
    
    // Public submission (Anyone can submit)
    std::string submitBlogPublic(const BlogPost& blog);
    std::vector<BlogPost> getSubmittedBlogs(int page = 1, int limit = 10);
    std::vector<BlogPost> getDraftBlogs(int page = 1, int limit = 10);
    std::vector<BlogPost> getPublishedBlogs(int page = 1, int limit = 10);
    
    // Get by status
    std::vector<BlogPost> getBlogsByStatus(BlogStatus status, int page = 1, int limit = 10);

private:
    mongocxx::collection getBlogsCollection();
};
