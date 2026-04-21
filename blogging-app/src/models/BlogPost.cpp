#include "models/BlogPost.h"
#include "utils/SlugGenerator.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
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

BlogPost BlogPost::fromBson(const bsoncxx::document::view& doc) {
    BlogPost blog;
    
    try {
        if (doc["_id"]) {
            blog.id = bsoncxx::to_json(doc["_id"].get_oid());
        }
        if (doc["title"]) {
            blog.title = std::string(doc["title"].get_string());
        }
        if (doc["subtitle"]) {
            blog.subtitle = std::string(doc["subtitle"].get_string());
        }
        if (doc["slug"]) {
            blog.slug = std::string(doc["slug"].get_string());
        }
        if (doc["content"]) {
            blog.content = std::string(doc["content"].get_string());
        }
        if (doc["author"]) {
            blog.author = std::string(doc["author"].get_string());
        }
        if (doc["category"]) {
            blog.category = std::string(doc["category"].get_string());
        }
        if (doc["status"]) {
            blog.status = blogStatusFromString(std::string(doc["status"].get_string()));
        }
        if (doc["views"]) {
            blog.views = doc["views"].get_int32();
        }
        if (doc["featured_image"]) {
            blog.featured_image = std::string(doc["featured_image"].get_string());
        }
        if (doc["created_at"]) {
            auto ms = doc["created_at"].get_date();
            blog.created_at = std::string(doc["created_at"].get_string());
        }
        if (doc["updated_at"]) {
            blog.updated_at = std::string(doc["updated_at"].get_string());
        }
        
        // Parse tags array
        if (doc["tags"]) {
            auto tags_array = doc["tags"].get_array();
            for (const auto& tag : tags_array.value) {
                blog.tags.push_back(std::string(tag.get_string()));
            }
        }
    } catch (const std::exception& e) {
        // Log error but continue
    }
    
    return blog;
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

bsoncxx::document::value BlogPost::toBson() const {
    using bsoncxx::builder::stream::document;
    using bsoncxx::builder::stream::open_array;
    using bsoncxx::builder::stream::close_array;
    using bsoncxx::builder::stream::finalize;
    
    bsoncxx::builder::stream::document doc{};
    
    if (!id.empty()) {
        doc << "_id" << id;
    }
    
    doc << "title" << title
        << "subtitle" << subtitle
        << "slug" << slug
        << "content" << content
        << "author" << author
        << "category" << category
        << "status" << blogStatusToString(status)
        << "views" << views
        << "featured_image" << featured_image
        << "created_at" << created_at
        << "updated_at" << updated_at
        << "tags" << open_array;
    
    for (const auto& tag : tags) {
        doc << tag;
    }
    
    doc << close_array;
    
    return doc << finalize;
}

std::string BlogPost::generateSlug(const std::string& title) {
    return SlugGenerator::generate(title);
}
