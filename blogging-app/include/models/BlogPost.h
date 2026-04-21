#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/json.hpp>

enum class BlogStatus {
    SUBMITTED = 0,  // Public submission
    APPROVED = 1,   // Admin approved
    DRAFT = 2,      // Admin drafted
    PUBLISHED = 3   // Live
};

std::string blogStatusToString(BlogStatus status);
BlogStatus blogStatusFromString(const std::string& status);

class BlogPost {
public:
    std::string id;
    std::string title;
    std::string subtitle;
    std::string slug;
    std::string content;
    std::vector<std::string> tags;
    std::string author;
    std::string category;
    BlogStatus status;  // submitted, approved, draft, published
    int views;
    std::string created_at;
    std::string updated_at;
    std::string featured_image;

    BlogPost() : status(BlogStatus::SUBMITTED), views(0) {}

    // Convert BSON document to BlogPost
    static BlogPost fromBson(const bsoncxx::document::view& doc);
    
    // Convert BlogPost to JSON string
    std::string toJson() const;
    
    // Convert BlogPost to BSON for database operations
    bsoncxx::document::value toBson() const;

    // Helper functions
    std::string generateSlug(const std::string& title);
};
