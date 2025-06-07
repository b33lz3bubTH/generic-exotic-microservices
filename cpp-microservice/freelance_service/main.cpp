#include <pistache/endpoint.h>
#include <pistache/router.h>
#include "controllers/UserController.h"

int main() {
    Pistache::Http::Endpoint server(Pistache::Address("*:9080"));
    auto opts = Pistache::Http::Endpoint::options().threads(1);
    server.init(opts);

    Pistache::Rest::Router router;
    UserController userController(router);

    server.setHandler(router.handler());
    server.serve();
    return 0;
} 