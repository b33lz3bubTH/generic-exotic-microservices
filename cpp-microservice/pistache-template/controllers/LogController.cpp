#include "LogController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace Pistache;

LogController::LogController(Rest::Router& r) : router(r) {
    setupRoutes();
}

void LogController::setupRoutes() {
    Rest::Routes::Post(router, "/logs", Rest::Routes::bind(&LogController::handleCreateLog, this));
    Rest::Routes::Get(router, "/logs/search", Rest::Routes::bind(&LogController::handleSearchLogs, this));
    Rest::Routes::Get(router, "/logs/list", Rest::Routes::bind(&LogController::handleGetLogsPaginated, this));
}

void LogController::handleCreateLog(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto data = json::parse(request.body());
        LogEntry log{0,
            data["message"].get<std::string>(),
            data.value("stackTrace", ""),
            data.value("meta", json::object()),
            "", // createdAt handled by DB
            data.value("createdBy", "")
        };
        repository.createLog(log);
        response.send(Http::Code::Created, R"({"message":"Log created", "status_code": "200"})", MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", e.what()}}).dump(), MIME(Application, Json));
    }
}

void LogController::handleSearchLogs(const Rest::Request& request, Http::ResponseWriter response) {
    auto q = request.query().get("q");
    if (!q) {
        response.send(Http::Code::Bad_Request, R"({"error":"Missing query parameter q"})", MIME(Application, Json));
        return;
    }
    auto logs = repository.searchLogs(*q);
    json arr = json::array();
    for (const auto& log : logs) arr.push_back(log.to_json());
    response.send(Http::Code::Ok, json({{"logs", arr}}).dump(), MIME(Application, Json));
}

void LogController::handleGetLogsPaginated(const Rest::Request& request, Http::ResponseWriter response) {
    int skip = 0, take = 10;
    if (auto s = request.query().get("skip")) skip = std::stoi(*s);
    if (auto t = request.query().get("take")) take = std::stoi(*t);
    auto logs = repository.getLogsPaginated(skip, take);
    int total = repository.getTotalCount();
    json arr = json::array();
    for (const auto& log : logs) arr.push_back(log.to_json());
    response.send(Http::Code::Ok, json({{"skip", skip}, {"take", take}, {"total", total}, {"logs", arr}}).dump(), MIME(Application, Json));
}
