#include "models/Image.h"

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/types.hpp>
#include <sstream>
#include <iostream>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

std::string imageStatusToString(ImageStatus status) {
    switch (status) {
        case ImageStatus::PENDING: return "pending";
        case ImageStatus::APPROVED: return "approved";
        case ImageStatus::REJECTED: return "rejected";
        case ImageStatus::NSFW_FLAGGED: return "nsfw_flagged";
        default: return "unknown";
    }
}

ImageStatus imageStatusFromString(const std::string& status) {
    if (status == "pending") return ImageStatus::PENDING;
    if (status == "approved") return ImageStatus::APPROVED;
    if (status == "rejected") return ImageStatus::REJECTED;
    if (status == "nsfw_flagged") return ImageStatus::NSFW_FLAGGED;
    return ImageStatus::PENDING;
}

Image Image::fromBson(const bsoncxx::document::view& doc) {
    Image image;
    try {
        if (doc["id"]) image.id = std::string(doc["id"].get_utf8().value);
        if (doc["album_id"]) image.album_id = std::string(doc["album_id"].get_utf8().value);
        if (doc["url"]) image.url = std::string(doc["url"].get_utf8().value);
        if (doc["alt_text"]) image.alt_text = std::string(doc["alt_text"].get_utf8().value);
        if (doc["caption"]) image.caption = std::string(doc["caption"].get_utf8().value);
        if (doc["display_order"]) image.display_order = doc["display_order"].get_int32().value;
        if (doc["status"]) image.status = imageStatusFromString(std::string(doc["status"].get_utf8().value));
        if (doc["nsfw_flagged"]) image.nsfw_flagged = doc["nsfw_flagged"].get_bool().value;
        if (doc["nsfw_reason"]) image.nsfw_reason = std::string(doc["nsfw_reason"].get_utf8().value);
        if (doc["created_at"]) image.created_at = std::string(doc["created_at"].get_utf8().value);
        if (doc["updated_at"]) image.updated_at = std::string(doc["updated_at"].get_utf8().value);
    } catch (const std::exception& e) {
        std::cerr << "Error converting BSON to Image: " << e.what() << std::endl;
    }
    return image;
}

std::string Image::toJson() const {
    std::stringstream ss;
    ss << "{"
       << "\"id\":\"" << id << "\"," 
       << "\"album_id\":\"" << album_id << "\"," 
       << "\"url\":\"" << url << "\"," 
       << "\"alt_text\":\"" << alt_text << "\"," 
       << "\"caption\":\"" << caption << "\"," 
       << "\"display_order\":" << display_order << ","
       << "\"status\":\"" << imageStatusToString(status) << "\"," 
       << "\"nsfw_flagged\":" << (nsfw_flagged ? "true" : "false") << ","
       << "\"nsfw_reason\":\"" << nsfw_reason << "\"," 
       << "\"created_at\":\"" << created_at << "\"," 
       << "\"updated_at\":\"" << updated_at << "\""
       << "}";
    return ss.str();
}

bsoncxx::document::value Image::toBson() const {
    return document{}
           << "id" << id
           << "album_id" << album_id
           << "url" << url
           << "alt_text" << alt_text
           << "caption" << caption
           << "display_order" << display_order
           << "status" << imageStatusToString(status)
           << "nsfw_flagged" << nsfw_flagged
           << "nsfw_reason" << nsfw_reason
           << "created_at" << created_at
           << "updated_at" << updated_at
           << finalize;
}

bool Image::validate() const {
    return !id.empty() && !url.empty() && !alt_text.empty();
}
