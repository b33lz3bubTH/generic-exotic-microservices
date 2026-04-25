#include "services/AlbumService.h"

#include <random>
#include <chrono>
#include <iostream>
#include <map>
#include <set>
#include <mutex>

// In-memory storage for now
static std::map<std::string, Album> albums;
static std::set<std::string> validTokens;
static std::mutex albumsMutex;

namespace {
void recomputeAlbumCounters(Album& album) {
    album.image_count = static_cast<int>(album.images.size());
    album.public_image_count = 0;
    album.pending_image_count = 0;
    album.rejected_image_count = 0;
    album.nsfw_image_count = 0;

    for (const auto& image : album.images) {
        switch (image.status) {
            case ImageStatus::APPROVED:
                ++album.public_image_count;
                break;
            case ImageStatus::REJECTED:
                ++album.rejected_image_count;
                break;
            case ImageStatus::NSFW_FLAGGED:
                ++album.nsfw_image_count;
                break;
            case ImageStatus::PENDING:
            default:
                ++album.pending_image_count;
                break;
        }
    }
}
}  // namespace

std::string AlbumService::nowEpochString() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

bool AlbumService::initialize(const std::string& connectionString) {
    // For now, just log that we're using in-memory storage
    std::cerr << "AlbumService: Using in-memory storage backend" << std::endl;
    return true;
}

std::string AlbumService::createAlbum(const Album& album) {
    std::lock_guard<std::mutex> lock(albumsMutex);
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
    std::lock_guard<std::mutex> lock(albumsMutex);
    Album empty;
    auto it = albums.find(id);
    if (it != albums.end()) {
        return it->second;
    }
    return empty;
}

Album AlbumService::getAlbumByToken(const std::string& token) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    Album empty;
    for (auto& pair : albums) {
        if (pair.second.admin_token == token) {
            return pair.second;
        }
    }
    return empty;
}

std::vector<Album> AlbumService::getAllAlbums(int page, int limit) {
    std::lock_guard<std::mutex> lock(albumsMutex);
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
    std::lock_guard<std::mutex> lock(albumsMutex);
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
    std::lock_guard<std::mutex> lock(albumsMutex);
    return albums.erase(id) > 0;
}

bool AlbumService::submitAlbum(const std::string& id) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    auto it = albums.find(id);
    if (it != albums.end() &&
        canTransitionAlbumStatus(it->second.status, AlbumStatus::SUBMITTED) &&
        it->second.image_count > 0) {
        it->second.status = AlbumStatus::SUBMITTED;
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

bool AlbumService::publishAlbum(const std::string& id) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    auto it = albums.find(id);
    if (it != albums.end() &&
        canTransitionAlbumStatus(it->second.status, AlbumStatus::PUBLISHED) &&
        it->second.public_image_count > 0 &&
        it->second.pending_image_count == 0) {
        it->second.status = AlbumStatus::PUBLISHED;
        it->second.published_at = nowEpochString();
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

bool AlbumService::archiveAlbum(const std::string& id) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    auto it = albums.find(id);
    if (it != albums.end() && canTransitionAlbumStatus(it->second.status, AlbumStatus::ARCHIVED)) {
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
    {
        std::lock_guard<std::mutex> lock(albumsMutex);
        validTokens.insert(token);
    }
    return token;
}

bool AlbumService::checkTokenValidity(const std::string& token) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    return validTokens.find(token) != validTokens.end();
}

bool AlbumService::validateAndConsumeToken(const std::string& token, const std::string& albumId) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    if (validTokens.find(token) != validTokens.end()) {
        auto it = albums.find(albumId);
        if (it != albums.end() &&
            it->second.admin_token == token &&
            !it->second.token_used &&
            it->second.status == AlbumStatus::DRAFT) {
            it->second.token_used = true;
            validTokens.erase(token);
            it->second.updated_at = nowEpochString();
            return true;
        }
    }
    return false;
}

bool AlbumService::addImageToAlbum(const std::string& albumId, const Image& image) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    auto it = albums.find(albumId);
    if (it != albums.end() && it->second.status == AlbumStatus::DRAFT && !it->second.token_used && it->second.canAddImage()) {
        it->second.images.push_back(image);
        recomputeAlbumCounters(it->second);
        it->second.updated_at = nowEpochString();
        return true;
    }
    return false;
}

bool AlbumService::updateImageStatus(const std::string& albumId,
                                     const std::string& imageId,
                                     ImageStatus status,
                                     const std::string& nsfwReason) {
    std::lock_guard<std::mutex> lock(albumsMutex);
    auto it = albums.find(albumId);
    if (it != albums.end() && it->second.status != AlbumStatus::DRAFT) {
        for (auto& img : it->second.images) {
            if (img.id == imageId) {
                if (!canTransitionImageStatus(img.status, status)) {
                    return false;
                }
                img.status = status;
                img.nsfw_flagged = (status == ImageStatus::NSFW_FLAGGED);
                if (!nsfwReason.empty()) {
                    img.nsfw_reason = nsfwReason;
                }
                img.updated_at = nowEpochString();
                recomputeAlbumCounters(it->second);
                it->second.updated_at = nowEpochString();
                return true;
            }
        }
    }
    return false;
}
