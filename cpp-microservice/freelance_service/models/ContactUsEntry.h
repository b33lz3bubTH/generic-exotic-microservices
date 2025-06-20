// Model for storing contact us form submissions from multiple sites/apps.
// Follows scalable, flexible JSON structure for extras field.
#pragma once
#include <string>
#include <nlohmann/json.hpp>

class ContactUsEntry {
public:
    std::string uuid;
    std::string site;
    std::string type;
    std::string message;
    nlohmann::json extras;
    std::string createdAt;

    ContactUsEntry() = default;
    ContactUsEntry(const std::string& uuid, const std::string& site, const std::string& type,
                   const std::string& message, const nlohmann::json& extras, const std::string& createdAt);

    nlohmann::json to_json() const;
}; 