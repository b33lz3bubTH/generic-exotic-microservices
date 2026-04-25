#include "models/Image.h"

#include <sstream>

std::string imageStatusToString(ImageStatus status) {
    switch (status) {
        case ImageStatus::PENDING:
            return "pending";
        case ImageStatus::APPROVED:
            return "approved";
        case ImageStatus::REJECTED:
            return "rejected";
        case ImageStatus::NSFW_FLAGGED:
            return "nsfw_flagged";
        default:
            return "unknown";
    }
}

ImageStatus imageStatusFromString(const std::string& status) {
    if (status == "approved") return ImageStatus::APPROVED;
    if (status == "rejected") return ImageStatus::REJECTED;
    if (status == "nsfw_flagged") return ImageStatus::NSFW_FLAGGED;
    return ImageStatus::PENDING;
}

bool canTransitionImageStatus(ImageStatus from, ImageStatus to) {
    if (from == to) return true;

    switch (from) {
        case ImageStatus::PENDING:
            return to == ImageStatus::APPROVED || to == ImageStatus::REJECTED || to == ImageStatus::NSFW_FLAGGED;
        case ImageStatus::APPROVED:
            return to == ImageStatus::REJECTED || to == ImageStatus::NSFW_FLAGGED;
        case ImageStatus::REJECTED:
            return to == ImageStatus::APPROVED || to == ImageStatus::NSFW_FLAGGED;
        case ImageStatus::NSFW_FLAGGED:
            return to == ImageStatus::REJECTED;
        default:
            return false;
    }
}

std::string Image::toJson() const {
    std::ostringstream ss;
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

bool Image::validate() const {
    return !id.empty() && !album_id.empty() && !url.empty();
}
