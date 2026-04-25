#pragma once
#include <string>

class User {
public:
    int id;
    std::string username;
    std::string email;
    std::string password_hash;
    std::string display_name;
    std::string profile_image_url;
    std::string bio;
    std::string created_at;
    std::string updated_at;

    User() : id(-1) {}
    User(const std::string& username, const std::string& email, const std::string& passwordHash)
        : id(-1), username(username), email(email), password_hash(passwordHash) {}
    
    std::string toJson() const;
};
