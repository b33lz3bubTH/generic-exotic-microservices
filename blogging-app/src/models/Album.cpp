#include "models/Album.h"
#include <sstream>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/types.hpp>
#include <iostream>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::array;
using bsoncxx::builder::stream::finalize;

std::string albumStatusToString(AlbumStatus status) {
    switch (status) {
        case AlbumStatus::DRAFT: return "draft";
        case AlbumStatus::SUBMITTED: return "submitted";
        case AlbumStatus::APPROVED: return "approved";
        case AlbumStatus::PUBLISHED: return "published";
        case AlbumStatus::ARCHIVED: return "archived";
        default: return "unknown";
    }
}

AlbumStatus albumStatusFromString(const std::string& status) {
    if (status == "draft") return AlbumStatus::DRAFT;
    if (status == "submitted") return AlbumStatus::SUBMITTED;
    if (status == "approved") return AlbumStatus::APPROVED;
    if (status == "published") return AlbumStatus::PUBLISHED;
    if (status == "archived") return AlbumStatus::ARCHIVED;
    return AlbumStatus::DRAFT;
}

Album Album::fromBson(const bsoncxx::document::view& doc) {
    Album album;
    
    try {
        if (doc["_id"]) {
            album.id = bsoncxx::to_json(doc["_id"].get_document());
        }
        if (doc["title"]) {
            album.title = std::string(doc["title"].get_utf8().value);
        }
        if (doc["description"]) {
            album.description = std::string(doc["description"].get_utf8().value);
        }
        if (doc["admin_token"]) {
            album.admin_token = std::string(doc["admin_token"].get_utf8().value);
        }
        if (doc["status"]) {
            album.status = albumStatusFromString(std::string(doc["status"].get_utf8().value));
        }
        if (doc["image_count"]) {
            album.image_count = doc["image_count"].get_int32().value;
        }
        if (doc["public_image_count"]) {
            album.public_image_count = doc["public_image_count"].get_int32().value;
        }
        if (doc["uploader_name"]) {
            album.uploader_name = std::string(doc["uploader_name"].get_utf8().value);
        }
        if (doc["created_by"]) {
            album.created_by = std::string(doc["created_by"].get_utf8().value);
        }
        if (doc["created_at"]) {
            album.created_at = std::string(doc["created_at"].get_utf8().value);
        }
        if (doc["updated_at"]) {
            album.updated_at = std::string(doc["updated_at"].get_utf8().value);
        }
        if (doc["published_at"]) {
            album.published_at = std::string(doc["published_at"].get_utf8().value);
        }
        if (doc["token_used"]) {
            album.token_used = doc["token_used"].get_bool().value;
        }
        if (doc["images"] && doc["images"].type() == bsoncxx::type::k_array) {
            auto images_arr = doc["images"].get_array().value;
            for (auto&& elem : images_arr) {
                if (elem.type() == bsoncxx::type::k_document) {
                    album.images.push_back(Image::fromBson(elem.get_document().value));
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error converting BSON to Album: " << e.what() << std::endl;
    }
    
    return album;
}

std::string Album::toJson(bool include_all_images) const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":\"" << id << "\","
       << "\"title\":\"" << title << "\","
       << "\"description\":\"" << description << "\","
       << "\"status\":\"" << albumStatusToString(status) << "\","
       << "\"image_count\":" << image_count << ","
       << "\"public_image_count\":" << public_image_count << ","
       << "\"remaining_slots\":" << getRemainingSlots() << ","
       << "\"uploader_name\":\"" << uploader_name << "\","
       << "\"created_by\":\"" << created_by << "\","
       << "\"created_at\":\"" << created_at << "\","
       << "\"updated_at\":\"" << updated_at << "\","
       << "\"published_at\":\"" << published_at << "\"" 
    
    if (include_all_images && !images.empty()) {
        ss << ",\"images\":[";
        for (size_t i = 0; i < images.size(); ++i) {
            ss << images[i].toJson();
            if (i < images.size() - 1) ss << ",";
        }
        ss << "]";
    }
    
    ss << "}";
    return ss.str();
}

bsoncxx::document::value Album::toBson() const {
    auto images_array = array{};
    for (const auto& img : images) {
        images_array << img.toBson();
    }
    
    auto doc = document{}
        << "title" << title
        << "description" << description
        << "admin_token" << admin_token
        << "status" << albumStatusToString(status)
        << "image_count" << image_count
        << "public_image_count" << public_image_count
        << "uploader_name" << uploader_name
        << "created_by" << created_by
        << "created_at" << created_at
        << "updated_at" << updated_at
        << "published_at" << published_at
        << "token_used" << token_used
        << "images" << images_array
        << finalize;
    
    return doc;
}

bool Album::validate() const {
    return !title.empty() && !admin_token.empty();
}
