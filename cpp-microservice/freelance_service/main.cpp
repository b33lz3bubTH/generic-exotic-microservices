#include <pistache/endpoint.h>
#include <pistache/router.h>
#include "controllers/LogController.h"
#include "controllers/VaultController.h"
#include "controllers/ContactUsController.h"
#include "models/ServiceLogger.h"
#include <csignal>
#include <cstdlib>

using namespace Pistache;

Pistache::Http::Endpoint* serverPtr = nullptr;

void handleSignal(int signal) {
    ServiceLogger::log(LogLevel::INFO, "Received shutdown signal. Stopping server...");
    if (serverPtr) serverPtr->shutdown();
    Database::close();
    exit(0);
}

// Small HealthController for /health endpoint
class HealthController {
public:
    HealthController(Rest::Router& router) {
        Rest::Routes::Get(router, "/health", Rest::Routes::bind(&HealthController::health, this));
    }
    void health(const Rest::Request&, Http::ResponseWriter response) {
        response.send(Http::Code::Ok, R"({\"status\":\"ok\"})", MIME(Application, Json));
    }
};

int main() {
    // Read port from env or default
    int port = 9080;
    if (const char* env_p = std::getenv("PORT")) {
        port = std::atoi(env_p);
        if (port <= 0) port = 9080;
    }
    ServiceLogger::log(LogLevel::INFO, "Starting logger microservice on port " + std::to_string(port));
    Pistache::Http::Endpoint server(Pistache::Address("*:" + std::to_string(port)));
    serverPtr = &server;
    auto opts = Pistache::Http::Endpoint::options().threads(1);
    server.init(opts);

    Pistache::Rest::Router router;
    LogController logController(router);
    HealthController healthController(router);
    VaultController vaultController(router);
    ContactUsController contactUsController(router);

    // Set router as handler
    server.setHandler(router.handler());

    // Graceful shutdown
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    server.serve();
    Database::close();
    return 0;
}