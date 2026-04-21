#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include "../repositories/LogRepository.h"

class LogController {
public:
    LogController(Pistache::Rest::Router& router);

private:
    void setupRoutes();
    void handleCreateLog(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleSearchLogs(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleGetLogsPaginated(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);

    LogRepository repository;
    Pistache::Rest::Router& router;
};
