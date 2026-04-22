#include "services/AlbumService.h"

#include <bsoncxx/builder/stream/array.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/options/find.hpp>
#include <random>
#include <chrono>
#include <iostream>

using bsoncxx::builder::stream::array;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;
using bsoncxx::builder::stream::open_document;

std::shared_ptr<mongocxx::client> AlbumService::client = nullptr;
std::string AlbumService::dbName = "photo_sharing";
std::string AlbumService::albumsCollection = "albums";

bool AlbumService::initialize(const std::string& connectionString) {
    try {
        mongocxx::instance::current();
        client = std::make_shared<mongocxx::client>(mongocxx::uri(connectionString));
        auto pingCmd = document{} << "ping" << 1 << finalize;
        client->db(dbName).run_command(pingCmd.view());
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Mongo initialization failed: " << e.what() << std::endl;
        return false;
    }
}

std::string AlbumService::nowEpochString() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

std::string AlbumService::createAlbum(const Album& album) {
    try {
        if (!client) {
            return "";
        }

        auto coll = client->db(dbName)[albumsCollection];
        Album newAlbum = album;
        newAlbum.created_at = nowEpochString();
        newAlbum.updated_at = newAlbum.created_at;
        newAlbum.status = AlbumStatus::DRAFT;
        newAlbum.token_used = false;

        auto result = coll.insert_one(newAlbum.toBson().view());
        if (!result) {
            return "";
        }
        return result->inserted_id().get_oid().value.to_string();
    } catch (const std::exception& e) {
        std::cerr << "createAlbum failed: " << e.what() << std::endl;
        return "";
    }
}

Album AlbumService::getAlbumById(const std::string& id) {
    Album empty;
    try {
        if (!client || id.empty()) {
            return empty;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto doc = coll.find_one(document{} << "_id" << bsoncxx::oid(id) << finalize);
        if (!doc) {
            return empty;
        }

        return Album::fromBson(doc->view());
    } catch (const std::exception& e) {
        std::cerr << "getAlbumById failed: " << e.what() << std::endl;
        return empty;
    }
}

Album AlbumService::getAlbumByToken(const std::string& token) {
    Album empty;
    try {
        if (!client || token.empty()) {
            return empty;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto doc = coll.find_one(document{} << "admin_token" << token << finalize);
        if (!doc) {
            return empty;
        }

        return Album::fromBson(doc->view());
    } catch (const std::exception& e) {
        std::cerr << "getAlbumByToken failed: " << e.what() << std::endl;
        return empty;
    }
}

std::vector<Album> AlbumService::getAllAlbums(int page, int limit) {
    std::vector<Album> albums;
    try {
        if (!client) {
            return albums;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto opts = mongocxx::options::find{};
        opts.skip((std::max)(0, page - 1) * (std::max)(1, limit));
        opts.limit((std::max)(1, limit));

        auto cursor = coll.find({}, opts);
        for (const auto& doc : cursor) {
            albums.push_back(Album::fromBson(doc));
        }
        return albums;
    } catch (const std::exception& e) {
        std::cerr << "getAllAlbums failed: " << e.what() << std::endl;
        return albums;
    }
}

std::vector<Album> AlbumService::getAlbumsByStatus(AlbumStatus status, int page, int limit) {
    std::vector<Album> albums;
    try {
        if (!client) {
            return albums;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto opts = mongocxx::options::find{};
        opts.skip((std::max)(0, page - 1) * (std::max)(1, limit));
        opts.limit((std::max)(1, limit));

        auto cursor = coll.find(document{} << "status" << albumStatusToString(status) << finalize, opts);
        for (const auto& doc : cursor) {
            albums.push_back(Album::fromBson(doc));
        }
        return albums;
    } catch (const std::exception& e) {
        std::cerr << "getAlbumsByStatus failed: " << e.what() << std::endl;
        return albums;
    }
}

bool AlbumService::deleteAlbum(const std::string& id) {
    try {
        if (!client || id.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto result = coll.delete_one(document{} << "_id" << bsoncxx::oid(id) << finalize);
        return result && result->deleted_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "deleteAlbum failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::submitAlbum(const std::string& id) {
    try {
        if (!client || id.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        const auto now = nowEpochString();

        auto filter = document{}
                      << "_id" << bsoncxx::oid(id)
                      << "status" << albumStatusToString(AlbumStatus::DRAFT)
                      << "image_count" << open_document << "$gt" << 0 << close_document
                      << finalize;

        auto update = document{}
                      << "$set" << open_document
                      << "status" << albumStatusToString(AlbumStatus::SUBMITTED)
                      << "updated_at" << now
                      << close_document
                      << finalize;

        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "submitAlbum failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::publishAlbum(const std::string& id) {
    try {
        if (!client || id.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        const auto now = nowEpochString();

        auto update = document{}
                      << "$set" << open_document
                      << "status" << albumStatusToString(AlbumStatus::PUBLISHED)
                      << "published_at" << now
                      << "updated_at" << now
                      << close_document
                      << finalize;

        auto result = coll.update_one(document{} << "_id" << bsoncxx::oid(id) << finalize, update.view());
        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "publishAlbum failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::archiveAlbum(const std::string& id) {
    try {
        if (!client || id.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto update = document{}
                      << "$set" << open_document
                      << "status" << albumStatusToString(AlbumStatus::ARCHIVED)
                      << "updated_at" << nowEpochString()
                      << close_document
                      << finalize;

        auto result = coll.update_one(document{} << "_id" << bsoncxx::oid(id) << finalize, update.view());
        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "archiveAlbum failed: " << e.what() << std::endl;
        return false;
    }
}

std::string AlbumService::generateAdminToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 15);

    std::string token;
    token.reserve(32);
    static const char* hex = "0123456789abcdef";
    for (int i = 0; i < 32; ++i) {
        token.push_back(hex[dist(gen)]);
    }
    return token;
}

bool AlbumService::checkTokenValidity(const std::string& token) {
    try {
        if (!client || token.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto doc = coll.find_one(document{}
                                 << "admin_token" << token
                                 << "token_used" << false
                                 << "status" << albumStatusToString(AlbumStatus::DRAFT)
                                 << finalize);
        return doc.has_value();
    } catch (const std::exception& e) {
        std::cerr << "checkTokenValidity failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::validateAndConsumeToken(const std::string& token, const std::string& albumId) {
    try {
        if (!client || token.empty() || albumId.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto result = coll.update_one(
            document{}
                << "_id" << bsoncxx::oid(albumId)
                << "admin_token" << token
                << "token_used" << false
                << finalize,
            document{}
                << "$set" << open_document
                << "token_used" << true
                << "updated_at" << nowEpochString()
                << close_document
                << finalize);

        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "validateAndConsumeToken failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::addImageToAlbum(const std::string& albumId, const Image& image) {
    try {
        if (!client || albumId.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto now = nowEpochString();

        auto filter = document{}
                      << "_id" << bsoncxx::oid(albumId)
                      << "status" << albumStatusToString(AlbumStatus::DRAFT)
                      << "image_count" << open_document << "$lt" << 40 << close_document
                      << finalize;

        auto update = document{}
                      << "$push" << open_document << "images" << image.toBson().view() << close_document
                      << "$inc" << open_document << "image_count" << 1 << close_document
                      << "$set" << open_document << "updated_at" << now << close_document
                      << finalize;

        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "addImageToAlbum failed: " << e.what() << std::endl;
        return false;
    }
}

bool AlbumService::updateImageStatus(const std::string& albumId,
                                     const std::string& imageId,
                                     ImageStatus status,
                                     const std::string& nsfwReason) {
    try {
        if (!client || albumId.empty() || imageId.empty()) {
            return false;
        }

        auto coll = client->db(dbName)[albumsCollection];
        auto now = nowEpochString();

        auto filter = document{}
                      << "_id" << bsoncxx::oid(albumId)
                      << "images.id" << imageId
                      << finalize;

        auto updateBuilder = document{};
        updateBuilder << "$set" << open_document
                      << "images.$.status" << imageStatusToString(status)
                      << "images.$.updated_at" << now
                      << "updated_at" << now;

        if (status == ImageStatus::NSFW_FLAGGED) {
            updateBuilder << "images.$.nsfw_flagged" << true
                          << "images.$.nsfw_reason" << nsfwReason;
        }

        updateBuilder << close_document;

        if (status == ImageStatus::APPROVED) {
            updateBuilder << "$inc" << open_document << "public_image_count" << 1 << close_document;
        }

        auto update = updateBuilder << finalize;

        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() == 1;
    } catch (const std::exception& e) {
        std::cerr << "updateImageStatus failed: " << e.what() << std::endl;
        return false;
    }
}
