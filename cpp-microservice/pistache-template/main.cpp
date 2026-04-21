#include <pistache/endpoint.h>
#include <pistache/router.h>
#include "controllers/LogController.h"
#include "controllers/VaultController.h"
#include "controllers/ContactUsController.h"
#include "controllers/UserController.h"
#include "models/ServiceLogger.h"
#include "plugins/PluginManager.h"
#include "plugs/RackSackPlugin.h"
#include <csignal>
#include <cstdlib>
#include <thread>

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
    UserController userController(router);

    // Set router as handler
    server.setHandler(router.handler());

    // Graceful shutdown
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // Plugin system demo: run in a separate thread
    PluginManager pluginManager;
    pluginManager.registerPlugin(std::make_shared<RackSackPlugin>());
    // Inject database pointer to all plugins
    sqlite3* db = Database::getInstance(DatabaseUtils::DB_NAME);
    pluginManager.setDatabaseForAll(db);
    std::thread pluginThread([&pluginManager]() {
        try {
            pluginManager.runAll(); // or runAllParallel();
        } catch (const std::exception& e) {
            ServiceLogger::log(LogLevel::ERROR, std::string("Plugin error: ") + e.what());
        }
    });

    // Start the server (blocking)
    server.serve();

    // Wait for plugin thread to finish
    if (pluginThread.joinable()) pluginThread.join();

    Database::close();
    return 0;
}