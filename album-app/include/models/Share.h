#pragma once
#include <string>

class Share {
public:
    int id;
    int album_id;
    std::string share_token;
    int shared_by_user_id;
    bool is_public;
    std::string expiry_date;
    std::string created_at;
    std::string share_url;  // Full URL for public access

    Share() : id(-1), album_id(-1), shared_by_user_id(-1), is_public(true) {}
    Share(int albumId, const std::string& token, int userId)
        : id(-1), album_id(albumId), share_token(token), shared_by_user_id(userId), is_public(true) {}
    
    std::string toJson() const;
};
