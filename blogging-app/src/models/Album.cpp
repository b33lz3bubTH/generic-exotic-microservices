#include "models/Album.h"

#include <sstream>
#include <iostream>

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
       << "\"published_at\":\"" << published_at << "\"";

    if (include_all_images) {
        ss << ",\"images\":[";
        for (size_t i = 0; i < images.size(); ++i) {
            ss << images[i].toJson();
            if (i + 1 < images.size()) ss << ",";
        }
        ss << "]";
    }

    ss << "}";
    return ss.str();
}

bool Album::validate() const {
    return !title.empty() && !admin_token.empty();
}
