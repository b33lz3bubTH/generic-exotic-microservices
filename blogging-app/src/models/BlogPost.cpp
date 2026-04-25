#include "models/BlogPost.h"
#include "utils/SlugGenerator.h"
#include <json/json.h>
#include <sstream>
#include <iomanip>

std::string blogStatusToString(BlogStatus status) {
    switch (status) {
        case BlogStatus::SUBMITTED: return "submitted";
        case BlogStatus::APPROVED: return "approved";
        case BlogStatus::DRAFT: return "draft";
        case BlogStatus::PUBLISHED: return "published";
        default: return "submitted";
    }
}

BlogStatus blogStatusFromString(const std::string& status) {
    if (status == "approved") return BlogStatus::APPROVED;
    if (status == "draft") return BlogStatus::DRAFT;
    if (status == "published") return BlogStatus::PUBLISHED;
    return BlogStatus::SUBMITTED;
}

std::string BlogPost::toJson() const {
    Json::Value root(Json::objectValue);
    
    root["id"] = id;
    root["title"] = title;
    root["subtitle"] = subtitle;
    root["slug"] = slug;
    root["content"] = content;
    root["author"] = author;
    root["category"] = category;
    root["status"] = blogStatusToString(status);
    root["views"] = views;
    root["featured_image"] = featured_image;
    root["created_at"] = created_at;
    root["updated_at"] = updated_at;
    
    Json::Value tagsArray(Json::arrayValue);
    for (const auto& tag : tags) {
        tagsArray.append(tag);
    }
    root["tags"] = tagsArray;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    return Json::writeString(builder, root);
}

std::string BlogPost::generateSlug(const std::string& title) {
    return SlugGenerator::generate(title);
}
