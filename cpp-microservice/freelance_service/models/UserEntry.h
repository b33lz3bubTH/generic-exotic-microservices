#pragma once
#include <string>
#include <nlohmann/json.hpp>

class UserEntry {
public:
    std::string id;
    std::string tenantId;
    std::string email;
    std::string password;
    std::string name;
    std::string phone;

    UserEntry() = default;
    UserEntry(const std::string& id, const std::string& tenantId, const std::string& email, const std::string& password,
              const std::string& name, const std::string& phone)
        : id(id), tenantId(tenantId), email(email), password(password), name(name), phone(phone) {}

    nlohmann::json to_json() const {
        return nlohmann::json{
            {"id", id},
            {"tenantId", tenantId},
            {"email", email},
            {"password", password},
            {"name", name},
            {"phone", phone}
        };
    }
}; 