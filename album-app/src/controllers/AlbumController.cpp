#include "controllers/AlbumController.h"

#include <json/json.h>
#include <openssl/evp.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_map>

#include "services/AlbumService.h"
#include "services/ImageService.h"

using namespace Pistache;

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

std::string toBody(const Json::Value& value) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    return Json::writeString(builder, value);
}

std::string nowMs() {
    const auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

std::string generateImageId() {
    static std::atomic<uint64_t> counter{0};
    return "img_" + nowMs() + "_" + std::to_string(++counter);
}

bool parseJsonBody(const std::string& body, Json::Value& output) {
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream stream(body);
    return Json::parseFromStream(builder, stream, &output, &errs);
}

std::string base64Decode(const std::string& input) {
    std::string normalized = input;
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\n'), normalized.end());
    normalized.erase(std::remove(normalized.begin(), normalized.end(), '\r'), normalized.end());

    std::string decoded;
    decoded.resize((normalized.size() * 3) / 4 + 1);
    const int outLen = EVP_DecodeBlock(reinterpret_cast<unsigned char*>(&decoded[0]),
                                       reinterpret_cast<const unsigned char*>(normalized.data()),
                                       static_cast<int>(normalized.size()));
    if (outLen < 0) {
        return "";
    }

    size_t padding = 0;
    if (!normalized.empty() && normalized.back() == '=') padding++;
    if (normalized.size() > 1 && normalized[normalized.size() - 2] == '=') padding++;

    decoded.resize(static_cast<size_t>(outLen) - padding);
    return decoded;
}
}  // namespace

AlbumController::AlbumController(Address address)
    : httpEndpoint(std::make_shared<Http::Endpoint>(address)) {}

void AlbumController::init(size_t threads) {
    auto opts = Http::Endpoint::options().threads(static_cast<int>(threads)).flags(Tcp::Options::InstallSignalHandler);
    httpEndpoint->init(opts);
    setupRoutes();
}

void AlbumController::start() {
    httpEndpoint->setHandler(router.handler());
    httpEndpoint->serve();
}

void AlbumController::shutdown() {
    httpEndpoint->shutdown();
}

void AlbumController::setupRoutes() {
    using namespace Rest;

    Routes::Get(router, "/api/albums", Routes::bind(&AlbumController::getPublishedAlbums, this));
    Routes::Get(router, "/api/albums/:id", Routes::bind(&AlbumController::getAlbumById, this));
    Routes::Get(router, "/api/albums/stats/overview", Routes::bind(&AlbumController::getAlbumStats, this));

    Routes::Post(router, "/api/admin/albums", Routes::bind(&AlbumController::createAlbum, this));
    Routes::Get(router, "/api/admin/albums", Routes::bind(&AlbumController::getAllAlbums, this));
    Routes::Get(router, "/api/admin/albums/:id", Routes::bind(&AlbumController::getAlbumDetailsAdmin, this));
    Routes::Delete(router, "/api/admin/albums/:id", Routes::bind(&AlbumController::deleteAlbum, this));
    Routes::Post(router, "/api/admin/albums/:id/publish", Routes::bind(&AlbumController::publishAlbum, this));
    Routes::Post(router, "/api/admin/albums/:id/archive", Routes::bind(&AlbumController::archiveAlbum, this));

    Routes::Post(router, "/api/admin/albums/submit", Routes::bind(&AlbumController::submitAlbumDraft, this));
    Routes::Post(router, "/api/admin/images/upload", Routes::bind(&AlbumController::uploadImagesToAlbum, this));
    Routes::Post(router, "/api/admin/images/:id/approve", Routes::bind(&AlbumController::approveImage, this));
    Routes::Post(router, "/api/admin/images/:id/reject", Routes::bind(&AlbumController::rejectImage, this));
    Routes::Post(router, "/api/admin/images/:id/nsfw", Routes::bind(&AlbumController::flagImageNSFW, this));
}

bool AlbumController::verifyAdminKey(const Rest::Request& req) const {
    const char* adminKey = std::getenv("PHOTO_ADMIN_KEY");
    const std::string expected = adminKey == nullptr ? "change-me-admin-key" : adminKey;
    auto header = req.headers().tryGetRaw("X-Admin-Key");
    return header.has_value() && header.value() == expected;
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

    if (entry.second >= 8) {
        return false;
    }

    ++entry.second;
    return true;
}

void AlbumController::getPublishedAlbums(const Rest::Request& req, Http::ResponseWriter response) {
    int page = req.query().get("page").isEmpty() ? 1 : std::stoi(req.query().get("page").get());
    int limit = req.query().get("limit").isEmpty() ? 10 : std::stoi(req.query().get("limit").get());

    auto albums = AlbumService::getAlbumsByStatus(AlbumStatus::PUBLISHED, page, limit);
    Json::Value data(Json::arrayValue);
    for (const auto& album : albums) {
        Json::Value parsed;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream s(album.toJson(false));
        if (Json::parseFromStream(builder, s, &parsed, &errs)) {
            data.append(parsed);
        }
    }

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Published albums retrieved", data)), MIME(Application, Json));
}

void AlbumController::getAlbumById(const Rest::Request& req, Http::ResponseWriter response) {
    const auto id = req.param(":id").as<std::string>();
    auto album = AlbumService::getAlbumById(id);
    if (album.id.empty() || album.status != AlbumStatus::PUBLISHED) {
        response.send(Http::Code::Not_Found, toBody(createJsonResponse(false, "Album not found")), MIME(Application, Json));
        return;
    }

    album.images.erase(
        std::remove_if(album.images.begin(), album.images.end(), [](const Image& img) { return img.status != ImageStatus::APPROVED; }),
        album.images.end());

    Json::Value parsed;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(album.toJson(true));
    Json::parseFromStream(builder, s, &parsed, &errs);

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Album retrieved", parsed)), MIME(Application, Json));
}

void AlbumController::getAlbumStats(const Rest::Request&, Http::ResponseWriter response) {
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

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Album statistics", stats)), MIME(Application, Json));
}

void AlbumController::createAlbum(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    Json::Value json;
    if (!parseJsonBody(req.body(), json) || !json.isMember("title") || json["title"].asString().empty()) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "title is required")), MIME(Application, Json));
        return;
    }

    Album album;
    album.title = json["title"].asString();
    album.description = json.isMember("description") ? json["description"].asString() : "";
    album.uploader_name = json.isMember("uploader_name") ? json["uploader_name"].asString() : "";
    album.admin_token = AlbumService::generateAdminToken();
    album.status = AlbumStatus::DRAFT;

    auto id = AlbumService::createAlbum(album);
    if (id.empty()) {
        response.send(Http::Code::Internal_Server_Error, toBody(createJsonResponse(false, "Failed to create album")), MIME(Application, Json));
        return;
    }

    Json::Value data;
    data["id"] = id;
    data["token"] = album.admin_token;
    data["max_images"] = 40;

    response.send(Http::Code::Created, toBody(createJsonResponse(true, "Album created", data)), MIME(Application, Json));
}

void AlbumController::uploadImagesToAlbum(const Rest::Request& req, Http::ResponseWriter response) {
    Json::Value json;
    if (!parseJsonBody(req.body(), json)) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "Invalid JSON body")), MIME(Application, Json));
        return;
    }

    const std::string token = json.isMember("token") ? json["token"].asString() : "";
    const std::string albumId = json.isMember("album_id") ? json["album_id"].asString() : "";

    if (token.empty() || albumId.empty()) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "album_id and token are required")), MIME(Application, Json));
        return;
    }

    auto client = req.address().host();
    if (!rateLimitUpload(client) || !rateLimitUpload(token)) {
        response.send(Http::Code::Too_Many_Requests, toBody(createJsonResponse(false, "Rate limit exceeded")), MIME(Application, Json));
        return;
    }

    if (!AlbumService::checkTokenValidity(token)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Invalid or used token")), MIME(Application, Json));
        return;
    }

    auto album = AlbumService::getAlbumById(albumId);
    if (album.id.empty() || album.admin_token != token || album.status != AlbumStatus::DRAFT) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "Album not found or not in draft state")), MIME(Application, Json));
        return;
    }

    if (!json.isMember("images") || !json["images"].isArray() || json["images"].empty()) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "images[] is required")), MIME(Application, Json));
        return;
    }

    std::filesystem::create_directories(ImageService::getStoragePath());

    Json::Value uploaded(Json::arrayValue);
    int order = album.image_count + 1;
    for (const auto& fileData : json["images"]) {
        if (!album.canAddImage()) {
            break;
        }

        const std::string fileName = fileData.isMember("file_name") ? fileData["file_name"].asString() : "";
        const std::string contentBase64 = fileData.isMember("content_base64") ? fileData["content_base64"].asString() : "";

        if (fileName.empty() || contentBase64.empty() || !ImageService::isAllowedFormat(fileName)) {
            continue;
        }

        const std::string decoded = base64Decode(contentBase64);
        if (decoded.empty()) {
            continue;
        }

        const std::string imageId = generateImageId();
        const std::string savedPath = ImageService::getStoragePath() + "/" + albumId + "_" + imageId + "_" + fileName;
        std::ofstream out(savedPath, std::ios::binary);
        out.write(decoded.data(), static_cast<std::streamsize>(decoded.size()));
        out.close();

        Image image;
        image.id = imageId;
        image.album_id = albumId;
        image.url = savedPath;
        image.alt_text = fileData.isMember("alt_text") ? fileData["alt_text"].asString() : "uploaded image";
        image.caption = fileData.isMember("caption") ? fileData["caption"].asString() : "";
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
    data["remaining_slots"] = std::max(0, 40 - album.image_count);

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Images uploaded", data)), MIME(Application, Json));
}

void AlbumController::submitAlbumDraft(const Rest::Request& req, Http::ResponseWriter response) {
    Json::Value json;
    if (!parseJsonBody(req.body(), json) || !json.isMember("album_id") || !json.isMember("token")) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "album_id and token are required")), MIME(Application, Json));
        return;
    }

    const auto albumId = json["album_id"].asString();
    const auto token = json["token"].asString();

    if (!AlbumService::validateAndConsumeToken(token, albumId)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Invalid token, mismatched album, or token already used")), MIME(Application, Json));
        return;
    }

    if (!AlbumService::submitAlbum(albumId)) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "Album needs at least one image before submission")), MIME(Application, Json));
        return;
    }

    Json::Value data;
    data["album_id"] = albumId;
    data["status"] = "submitted";

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Album submitted", data)), MIME(Application, Json));
}

void AlbumController::getAllAlbums(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    int page = req.query().get("page").isEmpty() ? 1 : std::stoi(req.query().get("page").get());
    int limit = req.query().get("limit").isEmpty() ? 10 : std::stoi(req.query().get("limit").get());

    auto albums = AlbumService::getAllAlbums(page, limit);
    Json::Value data(Json::arrayValue);

    for (const auto& album : albums) {
        Json::Value parsed;
        Json::CharReaderBuilder builder;
        std::string errs;
        std::istringstream s(album.toJson(true));
        if (Json::parseFromStream(builder, s, &parsed, &errs)) {
            data.append(parsed);
        }
    }

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Albums retrieved", data)), MIME(Application, Json));
}

void AlbumController::getAlbumDetailsAdmin(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    const auto id = req.param(":id").as<std::string>();
    auto album = AlbumService::getAlbumById(id);
    if (album.id.empty()) {
        response.send(Http::Code::Not_Found, toBody(createJsonResponse(false, "Album not found")), MIME(Application, Json));
        return;
    }

    Json::Value parsed;
    Json::CharReaderBuilder builder;
    std::string errs;
    std::istringstream s(album.toJson(true));
    Json::parseFromStream(builder, s, &parsed, &errs);

    response.send(Http::Code::Ok, toBody(createJsonResponse(true, "Album detail", parsed)), MIME(Application, Json));
}

void AlbumController::approveImage(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    Json::Value json;
    if (!parseJsonBody(req.body(), json) || !json.isMember("album_id") || !json.isMember("image_id")) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "album_id and image_id are required")), MIME(Application, Json));
        return;
    }

    const bool ok = AlbumService::updateImageStatus(json["album_id"].asString(), json["image_id"].asString(), ImageStatus::APPROVED);
    response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request,
                  toBody(createJsonResponse(ok, ok ? "Image approved" : "Invalid image state transition")), MIME(Application, Json));
}

void AlbumController::rejectImage(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    Json::Value json;
    if (!parseJsonBody(req.body(), json) || !json.isMember("album_id") || !json.isMember("image_id")) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "album_id and image_id are required")), MIME(Application, Json));
        return;
    }

    const bool ok = AlbumService::updateImageStatus(json["album_id"].asString(), json["image_id"].asString(), ImageStatus::REJECTED);
    response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request,
                  toBody(createJsonResponse(ok, ok ? "Image rejected" : "Invalid image state transition")), MIME(Application, Json));
}

void AlbumController::flagImageNSFW(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    Json::Value json;
    if (!parseJsonBody(req.body(), json) || !json.isMember("album_id") || !json.isMember("image_id")) {
        response.send(Http::Code::Bad_Request, toBody(createJsonResponse(false, "album_id and image_id are required")), MIME(Application, Json));
        return;
    }

    const bool ok = AlbumService::updateImageStatus(
        json["album_id"].asString(),
        json["image_id"].asString(),
        ImageStatus::NSFW_FLAGGED,
        json.isMember("reason") ? json["reason"].asString() : "manual moderation");

    response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request,
                  toBody(createJsonResponse(ok, ok ? "Image flagged" : "Invalid image state transition")), MIME(Application, Json));
}

void AlbumController::publishAlbum(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    const auto id = req.param(":id").as<std::string>();
    const bool ok = AlbumService::publishAlbum(id);
    response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request,
                  toBody(createJsonResponse(ok, ok ? "Album published" : "Album must be submitted, fully moderated, and contain approved images")), MIME(Application, Json));
}

void AlbumController::archiveAlbum(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    const auto id = req.param(":id").as<std::string>();
    const bool ok = AlbumService::archiveAlbum(id);
    response.send(ok ? Http::Code::Ok : Http::Code::Bad_Request,
                  toBody(createJsonResponse(ok, ok ? "Album archived" : "Invalid album state transition")), MIME(Application, Json));
}

void AlbumController::deleteAlbum(const Rest::Request& req, Http::ResponseWriter response) {
    if (!verifyAdminKey(req)) {
        response.send(Http::Code::Unauthorized, toBody(createJsonResponse(false, "Unauthorized")), MIME(Application, Json));
        return;
    }

    const auto id = req.param(":id").as<std::string>();
    const bool ok = AlbumService::deleteAlbum(id);
    response.send(ok ? Http::Code::Ok : Http::Code::Not_Found,
                  toBody(createJsonResponse(ok, ok ? "Album deleted" : "Album not found")), MIME(Application, Json));
}
