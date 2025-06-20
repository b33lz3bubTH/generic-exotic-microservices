#include "ContactUsEntry.h"

ContactUsEntry::ContactUsEntry(const std::string& uuid, const std::string& site, const std::string& type,
                               const std::string& message, const nlohmann::json& extras, const std::string& createdAt)
    : uuid(uuid), site(site), type(type), message(message), extras(extras), createdAt(createdAt) {}

nlohmann::json ContactUsEntry::to_json() const {
    return {
        {"uuid", uuid},
        {"site", site},
        {"type", type},
        {"message", message},
        {"extras", extras},
        {"createdAt", createdAt}
    };
} 