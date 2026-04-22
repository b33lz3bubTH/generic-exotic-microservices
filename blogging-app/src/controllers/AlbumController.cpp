#include "controllers/AlbumController.h"
#include "services/AlbumService.h"
#include "services/ImageService.h"
#include <json/json.h>
#include <iostream>

using HttpStatusCode = drogon::HttpStatusCode;

// Helper function to create JSON response
Json::Value createJsonResponse(bool success, const std::string& message, const Json::Value& data = Json::Value()) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = data;
    }
    return response;
}

void AlbumController::registerRoutes(drogon::HttpAppFramework& app) {
    // PUBLIC ROUTES
    app->registerHandler("/api/albums",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            getPublishedAlbums(req, std::move(callback));
        },
        {drogon::Get});
    
    app->registerHandler("/api/albums/{id}",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
            getAlbumById(req, std::move(callback), id);
        },
        {drogon::Get});
    
    app->registerHandler("/api/albums/search",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            searchAlbums(req, std::move(callback));
        },
        {drogon::Get});
    
    app->registerHandler("/api/albums/stats/overview",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            getAlbumStats(req, std::move(callback));
        },
        {drogon::Get});
    
    // ADMIN ROUTES
    app->registerHandler("/api/admin/albums",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            createAlbum(req, std::move(callback));
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/albums",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            getAllAlbums(req, std::move(callback));
        },
        {drogon::Get});
    
    app->registerHandler("/api/admin/albums/{id}",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
            getAlbumDetailsAdmin(req, std::move(callback), id);
        },
        {drogon::Get});
    
    app->registerHandler("/api/admin/albums/{id}/publish",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
            publishAlbum(req, std::move(callback), id);
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/albums/{id}/archive",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
            archiveAlbum(req, std::move(callback), id);
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/albums/{id}",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback, const std::string& id) {
            deleteAlbum(req, std::move(callback), id);
        },
        {drogon::Delete});
    
    app->registerHandler("/api/admin/images/upload",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            uploadImagesToAlbum(req, std::move(callback));
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/images/{id}/approve",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            approveImage(req, std::move(callback));
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/images/{id}/reject",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            rejectImage(req, std::move(callback));
        },
        {drogon::Post});
    
    app->registerHandler("/api/admin/images/{id}/nsfw",
        [](const drogon::HttpRequestPtr& req, std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
            flagImageNSFW(req, std::move(callback));
        },
        {drogon::Post});
}

bool AlbumController::verifyAdminToken(const std::string& token) {
    return AlbumService::checkTokenValidity(token);
}

void AlbumController::getPublishedAlbums(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        int page = 1;
        int limit = 10;
        
        if (!req->getParameter("page").empty()) {
            page = std::stoi(req->getParameter("page"));
        }
        if (!req->getParameter("limit").empty()) {
            limit = std::stoi(req->getParameter("limit"));
        }
        
        auto albums = AlbumService::getAlbumsByStatus(AlbumStatus::PUBLISHED, page, limit);
        
        Json::Value data = Json::Value(Json::arrayValue);
        for (const auto& album : albums) {
            data.append(Json::Value(album.toJson(false)));
        }
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Published albums retrieved successfully", data)
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::getAlbumById(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const std::string& id) {
    try {
        auto album = AlbumService::getAlbumById(id);
        
        if (album.id.empty()) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Album not found")
            );
            response->setStatusCode(HttpStatusCode::k404NotFound);
            callback(response);
            return;
        }
        
        // For public view, only show approved images
        Album public_album = album;
        public_album.images.erase(
            std::remove_if(public_album.images.begin(), public_album.images.end(),
                [](const Image& img) { return img.status != ImageStatus::APPROVED; }),
            public_album.images.end()
        );
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Album retrieved successfully", Json::Value(public_album.toJson(true)))
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::searchAlbums(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        std::string query = req->getParameter("q");
        
        if (query.empty()) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Search query is required")
            );
            response->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(response);
            return;
        }
        
        // TODO: Implement search functionality
        Json::Value data = Json::Value(Json::arrayValue);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Search results", data)
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::getAlbumStats(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto all_albums = AlbumService::getAllAlbums(1, 1000);
        
        Json::Value stats;
        stats["total_albums"] = static_cast<int>(all_albums.size());
        stats["published_albums"] = 0;
        stats["total_images"] = 0;
        stats["public_images"] = 0;
        
        int published = 0;
        int total_images = 0;
        int public_images = 0;
        
        for (const auto& album : all_albums) {
            if (album.status == AlbumStatus::PUBLISHED) published++;
            total_images += album.image_count;
            public_images += album.public_image_count;
        }
        
        stats["published_albums"] = published;
        stats["total_images"] = total_images;
        stats["public_images"] = public_images;
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Album statistics", stats)
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::createAlbum(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto json = req->getJsonObject();
        
        if (!json || !(*json).isMember("title")) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Title is required")
            );
            response->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(response);
            return;
        }
        
        Album new_album;
        new_album.title = (*json)["title"].asString();
        new_album.description = (*json)["description"].asString();
        new_album.uploader_name = (*json)["uploader_name"].asString();  // Creator's name for credit
        new_album.admin_token = AlbumService::generateAdminToken();
        new_album.status = AlbumStatus::DRAFT;
        
        std::string album_id = AlbumService::createAlbum(new_album);
        
        if (album_id.empty()) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Failed to create album")
            );
            response->setStatusCode(HttpStatusCode::k500InternalServerError);
            callback(response);
            return;
        }
        
        Json::Value data;
        data["id"] = album_id;
        data["token"] = new_album.admin_token;
        data["message"] = "Use this token to upload images. It can only be used once.";
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Album created successfully", data)
        );
        response->setStatusCode(HttpStatusCode::k201Created);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::uploadImagesToAlbum(const drogon::HttpRequestPtr& req,
                                         std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        // TODO: Implement multipart file upload handler
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "File upload not yet implemented")
        );
        response->setStatusCode(HttpStatusCode::k501NotImplemented);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::getAllAlbums(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        int page = 1;
        int limit = 10;
        
        if (!req->getParameter("page").empty()) {
            page = std::stoi(req->getParameter("page"));
        }
        if (!req->getParameter("limit").empty()) {
            limit = std::stoi(req->getParameter("limit"));
        }
        
        auto albums = AlbumService::getAllAlbums(page, limit);
        
        Json::Value data = Json::Value(Json::arrayValue);
        for (const auto& album : albums) {
            data.append(Json::Value(album.toJson(false)));
        }
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "All albums retrieved successfully", data)
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::getAlbumDetailsAdmin(const drogon::HttpRequestPtr& req,
                                          std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                          const std::string& id) {
    try {
        auto album = AlbumService::getAlbumById(id);
        
        if (album.id.empty()) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Album not found")
            );
            response->setStatusCode(HttpStatusCode::k404NotFound);
            callback(response);
            return;
        }
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(true, "Album details retrieved successfully", Json::Value(album.toJson(true)))
        );
        response->setStatusCode(HttpStatusCode::k200OK);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::approveImage(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto json = req->getJsonObject();
        
        if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Album ID and Image ID are required")
            );
            response->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(response);
            return;
        }
        
        std::string album_id = (*json)["album_id"].asString();
        std::string image_id = (*json)["image_id"].asString();
        
        bool success = AlbumService::approveImage(album_id, image_id);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Image approved" : "Failed to approve image")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::rejectImage(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto json = req->getJsonObject();
        
        if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Album ID and Image ID are required")
            );
            response->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(response);
            return;
        }
        
        std::string album_id = (*json)["album_id"].asString();
        std::string image_id = (*json)["image_id"].asString();
        
        bool success = AlbumService::rejectImage(album_id, image_id);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Image rejected" : "Failed to reject image")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::flagImageNSFW(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    try {
        auto json = req->getJsonObject();
        
        if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
            auto response = drogon::HttpResponse::newHttpJsonResponse(
                createJsonResponse(false, "Album ID and Image ID are required")
            );
            response->setStatusCode(HttpStatusCode::k400BadRequest);
            callback(response);
            return;
        }
        
        std::string album_id = (*json)["album_id"].asString();
        std::string image_id = (*json)["image_id"].asString();
        std::string reason = (*json)["reason"].asString();
        
        bool success = AlbumService::flagImageNSFW(album_id, image_id, reason);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Image flagged as NSFW" : "Failed to flag image")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::publishAlbum(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                  const std::string& id) {
    try {
        bool success = AlbumService::publishAlbum(id);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Album published" : "Failed to publish album")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::archiveAlbum(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                  const std::string& id) {
    try {
        bool success = AlbumService::archiveAlbum(id);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Album archived" : "Failed to archive album")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}

void AlbumController::deleteAlbum(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 const std::string& id) {
    try {
        bool success = AlbumService::deleteAlbum(id);
        
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(success, success ? "Album deleted" : "Album not found")
        );
        response->setStatusCode(success ? HttpStatusCode::k200OK : HttpStatusCode::k404NotFound);
        callback(response);
    } catch (const std::exception& e) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, std::string("Error: ") + e.what())
        );
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
    }
}
