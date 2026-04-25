#include "models/Album.h"

#include <sstream>

std::string albumStatusToString(AlbumStatus status) {
    switch (status) {
        case AlbumStatus::DRAFT:
            return "draft";
        case AlbumStatus::SUBMITTED:
            return "submitted";
        case AlbumStatus::PUBLISHED:
            return "published";
        case AlbumStatus::ARCHIVED:
            return "archived";
        default:
            return "unknown";
    }
}

AlbumStatus albumStatusFromString(const std::string& status) {
    if (status == "submitted") return AlbumStatus::SUBMITTED;
    if (status == "published") return AlbumStatus::PUBLISHED;
    if (status == "archived") return AlbumStatus::ARCHIVED;
    return AlbumStatus::DRAFT;
}

bool canTransitionAlbumStatus(AlbumStatus from, AlbumStatus to) {
    if (from == to) return true;

    switch (from) {
        case AlbumStatus::DRAFT:
            return to == AlbumStatus::SUBMITTED;
        case AlbumStatus::SUBMITTED:
            return to == AlbumStatus::PUBLISHED || to == AlbumStatus::ARCHIVED;
        case AlbumStatus::PUBLISHED:
            return to == AlbumStatus::ARCHIVED;
        case AlbumStatus::ARCHIVED:
            return false;
        default:
            return false;
    }
}

std::string Album::toJson(bool include_all_images) const {
    std::ostringstream ss;
    ss << "{"
       << "\"id\":\"" << id << "\"," 
       << "\"title\":\"" << title << "\"," 
       << "\"description\":\"" << description << "\"," 
       << "\"status\":\"" << albumStatusToString(status) << "\"," 
       << "\"image_count\":" << image_count << ","
       << "\"public_image_count\":" << public_image_count << ","
       << "\"pending_image_count\":" << pending_image_count << ","
       << "\"rejected_image_count\":" << rejected_image_count << ","
       << "\"nsfw_image_count\":" << nsfw_image_count << ","
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
