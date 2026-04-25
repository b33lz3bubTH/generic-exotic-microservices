#include <drogon/drogon.h>

#include <iostream>

#include "controllers/AlbumController.h"
#include "services/AlbumService.h"
#include "services/ImageService.h"

int main() {
    try {
        if (!AlbumService::initialize("")) {
            std::cerr << "Failed to initialize album service.\n";
            return 1;
        }

        if (!ImageService::initialize()) {
            std::cerr << "Failed to initialize local image storage.\n";
            return 1;
        }

        auto& app = drogon::app();
        AlbumController::registerRoutes(app);

        app.registerPreSendingAdvice([](const drogon::HttpRequestPtr&, const drogon::HttpResponsePtr& resp) {
            resp->addHeader("Access-Control-Allow-Origin", "*");
            resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Admin-Key");
        });

        app.registerHandler("/api/{1}",
                             [](const drogon::HttpRequestPtr&, std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
                                 auto resp = drogon::HttpResponse::newHttpResponse();
                                 resp->setStatusCode(drogon::k200OK);
                                 cb(resp);
                             },
                             {drogon::Options});

        app.addListener("0.0.0.0", 8080);
        app.setThreadNum(4);

        std::cout << "Photo sharing service running on :8080" << std::endl;
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
