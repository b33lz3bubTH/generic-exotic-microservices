#include "controllers/AlbumController.h"

#include <json/json.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "services/AlbumService.h"
#include "services/ImageService.h"

using HttpStatusCode = drogon::HttpStatusCode;

namespace {
Json::Value createJsonResponse(bool success, const std::string& message, const Json::Value& data = Json::Value()) {
    Json::Value response;
    response["success"] = success;
    response["message"] = message;
    if (!data.empty()) {
        response["data"] = data;
    }
    return response;
}

std::string nowMs() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

std::string generateImageId() {
    static std::atomic<uint64_t> counter{0};
    return "img_" + nowMs() + "_" + std::to_string(++counter);
}
}  // namespace

void AlbumController::registerRoutes(drogon::HttpAppFramework& app) {
    app.registerHandler("/api/albums", &getPublishedAlbums, {drogon::Get});
    app.registerHandler("/api/albums/{id}", &getAlbumById, {drogon::Get});
    app.registerHandler("/api/albums/stats/overview", &getAlbumStats, {drogon::Get});

    app.registerHandler("/api/admin/albums", &createAlbum, {drogon::Post});
    app.registerHandler("/api/admin/albums", &getAllAlbums, {drogon::Get});
    app.registerHandler("/api/admin/albums/{id}", &getAlbumDetailsAdmin, {drogon::Get});
    app.registerHandler("/api/admin/albums/{id}", &deleteAlbum, {drogon::Delete});
    app.registerHandler("/api/admin/albums/{id}/publish", &publishAlbum, {drogon::Post});
    app.registerHandler("/api/admin/albums/{id}/archive", &archiveAlbum, {drogon::Post});

    app.registerHandler("/api/admin/albums/submit", &submitAlbumDraft, {drogon::Post});
    app.registerHandler("/api/admin/images/upload", &uploadImagesToAlbum, {drogon::Post});
    app.registerHandler("/api/admin/images/{id}/approve", &approveImage, {drogon::Post});
    app.registerHandler("/api/admin/images/{id}/reject", &rejectImage, {drogon::Post});
    app.registerHandler("/api/admin/images/{id}/nsfw", &flagImageNSFW, {drogon::Post});
}

bool AlbumController::verifyAdminKey(const drogon::HttpRequestPtr& req) {
    const char* adminKey = std::getenv("PHOTO_ADMIN_KEY");
    const std::string expected = adminKey == nullptr ? "change-me-admin-key" : adminKey;
    return req->getHeader("X-Admin-Key") == expected;
}

bool AlbumController::rateLimitUpload(const std::string& key) {
    static std::mutex mtx;
    static std::unordered_map<std::string, std::pair<long long, int>> bucket;

    const auto now = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();

    std::lock_guard<std::mutex> lock(mtx);
    auto& entry = bucket[key];
    if (entry.first != now) {
        entry.first = now;
        entry.second = 0;
    }

    // 8 upload operations/sec per key (replace with Redis in production for stateless scaling)
    if (entry.second >= 8) {
        return false;
    }

    ++entry.second;
    return true;
}

void AlbumController::getPublishedAlbums(const drogon::HttpRequestPtr& req,
                                         std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    int page = 1;
    int limit = 10;
    if (!req->getParameter("page").empty()) page = std::stoi(req->getParameter("page"));
    if (!req->getParameter("limit").empty()) limit = std::stoi(req->getParameter("limit"));

    auto albums = AlbumService::getAlbumsByStatus(AlbumStatus::PUBLISHED, page, limit);
    Json::Value data(Json::arrayValue);
    for (const auto& album : albums) {
        data.append(Json::Value::null);
        Json::CharReaderBuilder builder;
        std::istringstream s(album.toJson(false));
        Json::Value parsed;
        std::string errs;
        if (Json::parseFromStream(builder, s, &parsed, &errs)) data[data.size() - 1] = parsed;
    }

    auto response = drogon::HttpResponse::newHttpJsonResponse(
        createJsonResponse(true, "Published albums retrieved", data));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::getAlbumById(const drogon::HttpRequestPtr&,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    auto album = AlbumService::getAlbumById(id);
    if (album.id.empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Album not found"));
        response->setStatusCode(HttpStatusCode::k404NotFound);
        callback(response);
        return;
    }

    album.images.erase(
        std::remove_if(album.images.begin(), album.images.end(),
                       [](const Image& img) { return img.status != ImageStatus::APPROVED; }),
        album.images.end());

    Json::Value parsed;
    Json::CharReaderBuilder builder;
    std::istringstream s(album.toJson(true));
    std::string errs;
    Json::parseFromStream(builder, s, &parsed, &errs);

    auto response = drogon::HttpResponse::newHttpJsonResponse(
        createJsonResponse(true, "Album retrieved", parsed));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::getAlbumStats(const drogon::HttpRequestPtr&,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto allAlbums = AlbumService::getAllAlbums(1, 1000);
    int published = 0;
    int totalImages = 0;
    int publicImages = 0;
    for (const auto& album : allAlbums) {
        if (album.status == AlbumStatus::PUBLISHED) ++published;
        totalImages += album.image_count;
        publicImages += album.public_image_count;
    }

    Json::Value stats;
    stats["total_albums"] = static_cast<int>(allAlbums.size());
    stats["published_albums"] = published;
    stats["total_images"] = totalImages;
    stats["public_images"] = publicImages;

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Album statistics", stats));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::createAlbum(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("title") || (*json)["title"].asString().empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "title is required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    Album album;
    album.title = (*json)["title"].asString();
    album.description = (*json).isMember("description") ? (*json)["description"].asString() : "";
    album.uploader_name = (*json).isMember("uploader_name") ? (*json)["uploader_name"].asString() : "";
    album.admin_token = AlbumService::generateAdminToken();
    album.status = AlbumStatus::DRAFT;

    auto id = AlbumService::createAlbum(album);
    if (id.empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Failed to create album"));
        response->setStatusCode(HttpStatusCode::k500InternalServerError);
        callback(response);
        return;
    }

    Json::Value data;
    data["id"] = id;
    data["token"] = album.admin_token;
    data["max_images"] = 40;

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Album created", data));
    response->setStatusCode(HttpStatusCode::k201Created);
    callback(response);
}

void AlbumController::uploadImagesToAlbum(const drogon::HttpRequestPtr& req,
                                          std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    const std::string token = req->getParameter("token");
    const std::string albumId = req->getParameter("album_id");

    if (token.empty() || albumId.empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "album_id and token are required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    if (!rateLimitUpload(req->peerAddr().toIpPort()) || !rateLimitUpload(token)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Rate limit exceeded"));
        response->setStatusCode(HttpStatusCode::k429TooManyRequests);
        callback(response);
        return;
    }

    if (!AlbumService::checkTokenValidity(token)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Invalid or used token"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto album = AlbumService::getAlbumById(albumId);
    if (album.id.empty() || album.admin_token != token || album.status != AlbumStatus::DRAFT) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "Album not found or not in draft state"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    auto files = req->getUploadedFiles();
    if (files.empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "At least one image required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value uploaded(Json::arrayValue);
    int order = album.image_count + 1;

    for (const auto& file : files) {
        if (!album.canAddImage()) {
            break;
        }

        if (!ImageService::isAllowedFormat(file.getFileName())) {
            continue;
        }

        const std::string storedPath = ImageService::uploadImage(file.getFilePath(), file.getFileName());
        if (storedPath.empty()) {
            continue;
        }

        Image image;
        image.id = generateImageId();
        image.album_id = albumId;
        image.url = storedPath;
        image.alt_text = req->getParameter("alt_text").empty() ? "uploaded image" : req->getParameter("alt_text");
        image.caption = req->getParameter("caption");
        image.display_order = order++;
        image.status = ImageStatus::PENDING;
        image.created_at = nowMs();
        image.updated_at = image.created_at;

        if (!image.validate()) {
            continue;
        }

        if (AlbumService::addImageToAlbum(albumId, image)) {
            Json::Value img;
            img["id"] = image.id;
            img["url"] = image.url;
            uploaded.append(img);
            album.image_count += 1;
        }
    }

    Json::Value data;
    data["uploaded_count"] = static_cast<int>(uploaded.size());
    data["images"] = uploaded;
    data["album_status"] = "draft";
    data["remaining_slots"] = (std::max)(0, 40 - album.image_count);

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Images uploaded", data));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::submitAlbumDraft(const drogon::HttpRequestPtr& req,
                                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("album_id") || !(*json).isMember("token")) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "album_id and token are required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    const auto albumId = (*json)["album_id"].asString();
    const auto token = (*json)["token"].asString();

    if (!AlbumService::validateAndConsumeToken(token, albumId)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "Invalid token, mismatched album, or token already used"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    if (!AlbumService::submitAlbum(albumId)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "Album needs at least one image before submission"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    Json::Value data;
    data["album_id"] = albumId;
    data["status"] = "submitted";

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Album submitted", data));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::getAllAlbums(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    int page = 1;
    int limit = 10;
    if (!req->getParameter("page").empty()) page = std::stoi(req->getParameter("page"));
    if (!req->getParameter("limit").empty()) limit = std::stoi(req->getParameter("limit"));

    auto albums = AlbumService::getAllAlbums(page, limit);
    Json::Value data(Json::arrayValue);

    for (const auto& album : albums) {
        Json::Value parsed;
        Json::CharReaderBuilder builder;
        std::istringstream s(album.toJson(true));
        std::string errs;
        if (Json::parseFromStream(builder, s, &parsed, &errs)) {
            data.append(parsed);
        }
    }

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Albums retrieved", data));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::getAlbumDetailsAdmin(const drogon::HttpRequestPtr& req,
                                           std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                           const std::string& id) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto album = AlbumService::getAlbumById(id);
    if (album.id.empty()) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Album not found"));
        response->setStatusCode(HttpStatusCode::k404NotFound);
        callback(response);
        return;
    }

    Json::Value parsed;
    Json::CharReaderBuilder builder;
    std::istringstream s(album.toJson(true));
    std::string errs;
    Json::parseFromStream(builder, s, &parsed, &errs);

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(true, "Album detail", parsed));
    response->setStatusCode(HttpStatusCode::k200OK);
    callback(response);
}

void AlbumController::approveImage(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "album_id and image_id are required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    const bool ok = AlbumService::updateImageStatus((*json)["album_id"].asString(),
                                                    (*json)["image_id"].asString(),
                                                    ImageStatus::APPROVED);

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Image approved" : "Update failed"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
    callback(response);
}

void AlbumController::rejectImage(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "album_id and image_id are required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    const bool ok = AlbumService::updateImageStatus((*json)["album_id"].asString(),
                                                    (*json)["image_id"].asString(),
                                                    ImageStatus::REJECTED);

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Image rejected" : "Update failed"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
    callback(response);
}

void AlbumController::flagImageNSFW(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    auto json = req->getJsonObject();
    if (!json || !(*json).isMember("album_id") || !(*json).isMember("image_id")) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(
            createJsonResponse(false, "album_id and image_id are required"));
        response->setStatusCode(HttpStatusCode::k400BadRequest);
        callback(response);
        return;
    }

    const bool ok = AlbumService::updateImageStatus((*json)["album_id"].asString(),
                                                    (*json)["image_id"].asString(),
                                                    ImageStatus::NSFW_FLAGGED,
                                                    (*json).isMember("reason") ? (*json)["reason"].asString() : "manual moderation");

    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Image flagged" : "Update failed"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
    callback(response);
}

void AlbumController::publishAlbum(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    const bool ok = AlbumService::publishAlbum(id);
    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Album published" : "Publish failed"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
    callback(response);
}

void AlbumController::archiveAlbum(const drogon::HttpRequestPtr& req,
                                   std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                   const std::string& id) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    const bool ok = AlbumService::archiveAlbum(id);
    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Album archived" : "Archive failed"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k500InternalServerError);
    callback(response);
}

void AlbumController::deleteAlbum(const drogon::HttpRequestPtr& req,
                                  std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                  const std::string& id) {
    if (!verifyAdminKey(req)) {
        auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(false, "Unauthorized"));
        response->setStatusCode(HttpStatusCode::k401Unauthorized);
        callback(response);
        return;
    }

    const bool ok = AlbumService::deleteAlbum(id);
    auto response = drogon::HttpResponse::newHttpJsonResponse(createJsonResponse(ok, ok ? "Album deleted" : "Album not found"));
    response->setStatusCode(ok ? HttpStatusCode::k200OK : HttpStatusCode::k404NotFound);
    callback(response);
}
