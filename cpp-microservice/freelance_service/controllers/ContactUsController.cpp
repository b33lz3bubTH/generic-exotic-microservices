#include "ContactUsController.h"
#include <nlohmann/json.hpp>
#include <random>
#include <chrono>
#include <fstream>

using json = nlohmann::json;
using namespace Pistache;

// Generate a random alphanumeric string (length 32)
static std::string generate_random_id() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static thread_local std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    std::string s;
    for (int i = 0; i < 32; ++i) s += alphanum[dis(gen)];
    return s;
}

ContactUsController::ContactUsController(Rest::Router& r) : router(r), currentKeyIndex(0) {
    loadKeys();
    setupRoutes();
}

void ContactUsController::setupRoutes() {
    Rest::Routes::Put(router, "/contactus", Rest::Routes::bind(&ContactUsController::handleCreateEntry, this));
    Rest::Routes::Get(router, "/contactus/list", Rest::Routes::bind(&ContactUsController::handleGetEntriesPaginated, this));
}

void ContactUsController::handleCreateEntry(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto data = json::parse(request.body());
        std::string uuidStr = generate_random_id();
        ContactUsEntry entry{
            uuidStr,
            data["site"].get<std::string>(),
            data["type"].get<std::string>(),
            data["message"].get<std::string>(),
            data.value("extras", json::object()),
            "" // createdAt handled by DB
        };
        repository.createEntry(entry);
        response.send(Http::Code::Created, json({{"uuid", uuidStr}}).dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", e.what()}}).dump(), MIME(Application, Json));
    }
}

void ContactUsController::handleGetEntriesPaginated(const Rest::Request& request, Http::ResponseWriter response) {
    auto site = request.query().get("site");
    auto key = request.query().get("key");
    int skip = 0, take = 10;
    if (auto s = request.query().get("skip")) skip = std::stoi(*s);
    if (auto t = request.query().get("take")) take = std::stoi(*t);
    if (!site || !key) {
        response.send(Http::Code::Bad_Request, R"({"error":"Missing site or key parameter"})", MIME(Application, Json));
        return;
    }
    if (*key != "22rapist872broski22") {
        response.send(Http::Code::Unauthorized, R"({\"error\":\"Invalid key\"})", MIME(Application, Json));
        return;
    }
    
    auto entries = repository.getEntriesPaginated(*site, skip, take);
    int total = repository.getTotalCount(*site);
    json arr = json::array();
    for (const auto& entry : entries) arr.push_back(entry.to_json());
    response.send(Http::Code::Ok, json({{"skip", skip}, {"take", take}, {"total", total}, {"entries", arr}}).dump(), MIME(Application, Json));
}

void ContactUsController::loadKeys() {
    // For demo, hardcode 30 keys. In production, load from env/config file.
    keys.clear();
    for (int i = 0; i < 30; ++i) {
        keys.push_back("key" + std::to_string(i+1));
    }
    currentKeyIndex = 0;
}

bool ContactUsController::validateKey(const std::string& key, int& keyIndex, int& nextKeyIndex) {
    for (size_t i = 0; i < keys.size(); ++i) {
        if (keys[i] == key) {
            keyIndex = i;
            nextKeyIndex = (i + 1) % keys.size();
            currentKeyIndex = nextKeyIndex;
            return true;
        }
    }
    keyIndex = -1;
    nextKeyIndex = currentKeyIndex;
    return false;
} 