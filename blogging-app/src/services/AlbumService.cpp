#include "services/AlbumService.h"

#include <random>
#include <chrono>
#include <iostream>
#include <map>
#include <set>

// In-memory storage for now
static std::map<std::string, Album> albums;
static std::set<std::string> validTokens;

std::string AlbumService::nowEpochString() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

bool AlbumService::initialize(const std::string& connectionString) {
    // For now, just log that we're using in-memory storage
    std::cerr << "AlbumService: Using in-memory storage (not MongoDB)" << std::endl;
    return true;
}

std::string AlbumService::createAlbum(const Album& album) {
    std::string id = std::to_string(albums.size() + 1);
    Album newAlbum = album;
    newAlbum.id = id;
    newAlbum.created_at = nowEpochString();
    newAlbum.updated_at = newAlbum.created_at;
    newAlbum.status = AlbumStatus::DRAFT;
    newAlbum.token_used = false;
    
    albums[id] = newAlbum;
    return id;
}

Album AlbumService::getAlbumById(const std::string& id) {
    Album empty;
    auto it = albums.find(id);
    if (it != albums.end()) {
        return it->second;
    }
    return empty;
}

Album AlbumService::getAlbumByToken(const std::string& token) {
    Album empty;
    for (auto& pair : albums) {
        if (pair.second.admin_token == token) {
            return pair.second;
        }
    }
    return empty;
}

std::vector<Album> AlbumService::getAllAlbums(int page, int limit) {
    std::vector<Album> result;
    int skip = (page - 1) * limit;
    int count = 0;
    
    for (auto& pair : albums) {
        if (count >= skip && count < skip + limit) {
            result.push_back(pair.second);
        }
        count++;
    }
    return result;
}

std::vector<Album> AlbumService::getAlbumsByStatus(AlbumStatus status, int page, int limit) {
    std::vector<Album> result;
    int skip = (page - 1) * limit;
    int count = 0;
    
    for (auto& pair : albums) {
        if (pair.second.status == status) {
            if (count >= skip && count < skip + limit) {
                result.push_back(pair.second);
            }
            count++;
        }
    }
    return result;
}

bool AlbumService::deleteAlbum(const std::string& id) {
    return albums.erase(id) > 0;
}

bool AlbumService::submitAlbum(const std::string& id) {
    auto it = albums.find(id);
    if (it != albums.end() && it->second.status == AlbumStatus::DRAFT && it->second.image_count > 0) {
        it->second.status = AlbumStatus::SUBMITTED;
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

bool AlbumService::publishAlbum(const std::string& id) {
    auto it = albums.find(id);
    if (it != albums.end()) {
        it->second.status = AlbumStatus::PUBLISHED;
        it->second.published_at = nowEpochString();
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

bool AlbumService::archiveAlbum(const std::string& id) {
    auto it = albums.find(id);
    if (it != albums.end()) {
        it->second.status = AlbumStatus::ARCHIVED;
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

std::string AlbumService::generateAdminToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 15);

    std::string token;
    token.reserve(32);
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        token += hex[dist(gen)];
    }
    validTokens.insert(token);
    return token;
}

bool AlbumService::checkTokenValidity(const std::string& token) {
    return validTokens.find(token) != validTokens.end();
}

bool AlbumService::validateAndConsumeToken(const std::string& token, const std::string& albumId) {
    if (validTokens.find(token) != validTokens.end()) {
        auto it = albums.find(albumId);
        if (it != albums.end()) {
            it->second.token_used = true;
            validTokens.erase(token);
            return true;
        }
    }
    return false;
}

bool AlbumService::addImageToAlbum(const std::string& albumId, const Image& image) {
    auto it = albums.find(albumId);
    if (it != albums.end()) {
        it->second.images.push_back(image);
        it->second.image_count = it->second.images.size();
        return true;
    }
    return false;
}

bool AlbumService::updateImageStatus(const std::string& albumId,
                                     const std::string& imageId,
                                     ImageStatus status,
                                     const std::string& nsfwReason) {
    auto it = albums.find(albumId);
    if (it != albums.end()) {
        for (auto& img : it->second.images) {
            if (img.id == imageId) {
                img.status = status;
                if (!nsfwReason.empty()) {
                    img.nsfw_reason = nsfwReason;
                }
                return true;
            }
        }
    }
    return false;
}
