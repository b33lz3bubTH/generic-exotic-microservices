#include "services/AlbumService.h"
#include <mongocxx/instance.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <iostream>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::stream::finalize;

std::shared_ptr<mongocxx::client> AlbumService::client = nullptr;
std::string AlbumService::db_name = "photo_sharing";
std::string AlbumService::albums_collection = "albums";

bool AlbumService::initialize(const std::string& connection_string) {
    try {
        mongocxx::instance::current();
        mongocxx::uri uri(connection_string);
        client = std::make_shared<mongocxx::client>(uri);
        
        // Test connection
        auto ping_cmd = document{} << "ping" << 1 << finalize;
        client->db(db_name).run_command(ping_cmd.view());
        
        std::cout << "✓ Connected to MongoDB" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "✗ MongoDB connection failed: " << e.what() << std::endl;
        return false;
    }
}

std::string AlbumService::createAlbum(const Album& album) {
    try {
        if (!client) {
            std::cerr << "Database not initialized" << std::endl;
            return "";
        }
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        Album new_album = album;
        new_album.created_at = std::chrono::system_clock::now().time_since_epoch().count();
        new_album.updated_at = new_album.created_at;
        
        auto result = coll.insert_one(new_album.toBson().view());
        
        if (result) {
            return bsoncxx::to_json(result->inserted_id());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating album: " << e.what() << std::endl;
    }
    
    return "";
}

Album AlbumService::getAlbumById(const std::string& id) {
    Album empty;
    try {
        if (!client) return empty;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto doc = coll.find_one(document{} << "_id" << bsoncxx::oid(id) << finalize);
        
        if (doc) {
            return Album::fromBson(doc->view());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching album: " << e.what() << std::endl;
    }
    
    return empty;
}

Album AlbumService::getAlbumByToken(const std::string& token) {
    Album empty;
    try {
        if (!client) return empty;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto doc = coll.find_one(document{} << "admin_token" << token << finalize);
        
        if (doc) {
            return Album::fromBson(doc->view());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching album by token: " << e.what() << std::endl;
    }
    
    return empty;
}

std::vector<Album> AlbumService::getAllAlbums(int page, int limit) {
    std::vector<Album> albums;
    try {
        if (!client) return albums;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto cursor = coll.find({}, opts);
        
        for (auto&& doc : cursor) {
            albums.push_back(Album::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching albums: " << e.what() << std::endl;
    }
    
    return albums;
}

std::vector<Album> AlbumService::getAlbumsByStatus(AlbumStatus status, int page, int limit) {
    std::vector<Album> albums;
    try {
        if (!client) return albums;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto opts = mongocxx::options::find{};
        opts.skip((page - 1) * limit);
        opts.limit(limit);
        
        auto filter = document{} << "status" << albumStatusToString(status) << finalize;
        auto cursor = coll.find(filter.view(), opts);
        
        for (auto&& doc : cursor) {
            albums.push_back(Album::fromBson(doc));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error fetching albums by status: " << e.what() << std::endl;
    }
    
    return albums;
}

bool AlbumService::updateAlbum(const std::string& id, const Album& album) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document << "updated_at" 
                     << std::chrono::system_clock::now().time_since_epoch().count()
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error updating album: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::deleteAlbum(const std::string& id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto result = coll.delete_one(document{} << "_id" << bsoncxx::oid(id) << finalize);
        return result && result->deleted_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error deleting album: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::submitAlbum(const std::string& id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document 
                     << "status" << albumStatusToString(AlbumStatus::SUBMITTED)
                     << "updated_at" << std::chrono::system_clock::now().time_since_epoch().count()
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error submitting album: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::approveAlbum(const std::string& id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document 
                     << "status" << albumStatusToString(AlbumStatus::APPROVED)
                     << "updated_at" << std::chrono::system_clock::now().time_since_epoch().count()
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error approving album: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::publishAlbum(const std::string& id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        auto update = document{} << "$set" << open_document 
                     << "status" << albumStatusToString(AlbumStatus::PUBLISHED)
                     << "published_at" << now
                     << "updated_at" << now
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error publishing album: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::archiveAlbum(const std::string& id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(id) << finalize;
        auto update = document{} << "$set" << open_document 
                     << "status" << albumStatusToString(AlbumStatus::ARCHIVED)
                     << "updated_at" << std::chrono::system_clock::now().time_since_epoch().count()
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error archiving album: " << e.what() << std::endl;
    }
    
    return false;
}

std::string AlbumService::generateAdminToken() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 32; ++i) {
        ss << dis(gen);
    }
    
    return ss.str();
}

bool AlbumService::validateAndConsumeToken(const std::string& token) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "admin_token" << token << "token_used" << false << finalize;
        auto update = document{} << "$set" << open_document << "token_used" << true << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error validating token: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::checkTokenValidity(const std::string& token) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto doc = coll.find_one(document{} << "admin_token" << token << "token_used" << false << finalize);
        return doc.has_value();
    } catch (const std::exception& e) {
        std::cerr << "Error checking token: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::addImagesToAlbum(const std::string& album_id, const std::vector<Image>& images) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(album_id) << finalize;
        
        // Build images array
        auto images_array = bsoncxx::builder::stream::array{};
        for (const auto& img : images) {
            images_array << img.toBson();
        }
        
        auto update = document{} << "$push" << open_document 
                     << "images" << bsoncxx::builder::stream::array{} << images_array << close_document
                     << "$inc" << open_document << "image_count" << static_cast<int>(images.size())
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error adding images: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::approveImage(const std::string& album_id, const std::string& image_id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(album_id) 
                     << "images.id" << image_id << finalize;
        auto update = document{} << "$set" << open_document 
                     << "images.$.status" << imageStatusToString(ImageStatus::APPROVED)
                     << close_document
                     << "$inc" << open_document << "public_image_count" << 1 << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error approving image: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::rejectImage(const std::string& album_id, const std::string& image_id) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(album_id) 
                     << "images.id" << image_id << finalize;
        auto update = document{} << "$set" << open_document 
                     << "images.$.status" << imageStatusToString(ImageStatus::REJECTED)
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error rejecting image: " << e.what() << std::endl;
    }
    
    return false;
}

bool AlbumService::flagImageNSFW(const std::string& album_id, const std::string& image_id, const std::string& reason) {
    try {
        if (!client) return false;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto filter = document{} << "_id" << bsoncxx::oid(album_id) 
                     << "images.id" << image_id << finalize;
        auto update = document{} << "$set" << open_document 
                     << "images.$.status" << imageStatusToString(ImageStatus::NSFW_FLAGGED)
                     << "images.$.nsfw_flagged" << true
                     << "images.$.nsfw_reason" << reason
                     << close_document << finalize;
        
        auto result = coll.update_one(filter.view(), update.view());
        return result && result->modified_count() > 0;
    } catch (const std::exception& e) {
        std::cerr << "Error flagging NSFW image: " << e.what() << std::endl;
    }
    
    return false;
}

int AlbumService::getPublishedImageCount(const std::string& album_id) {
    try {
        if (!client) return 0;
        
        auto db = client->db(db_name);
        auto coll = db[albums_collection];
        
        auto doc = coll.find_one(document{} << "_id" << bsoncxx::oid(album_id) << finalize);
        
        if (doc && doc->view()["public_image_count"]) {
            return doc->view()["public_image_count"].get_int32().value;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting published image count: " << e.what() << std::endl;
    }
    
    return 0;
}
