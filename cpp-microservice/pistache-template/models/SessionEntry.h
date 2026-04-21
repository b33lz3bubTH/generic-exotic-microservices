#pragma once
#include <string>
#include <nlohmann/json.hpp>

class SessionEntry {
public:
    std::string id;
    std::string userId;
    std::string expiresAt;
    nlohmann::json userDetails;

    SessionEntry() = default;
    SessionEntry(const std::string& id, const std::string& userId, const std::string& expiresAt, const nlohmann::json& userDetails)
        : id(id), userId(userId), expiresAt(expiresAt), userDetails(userDetails) {}

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"userId", userId},
            {"expiresAt", expiresAt},
            {"userDetails", userDetails}
        };
    }
}; 