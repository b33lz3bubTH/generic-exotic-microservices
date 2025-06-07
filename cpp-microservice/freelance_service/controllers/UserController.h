#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/endpoint.h>
#include "../repositories/UserRepository.h"

class UserController {
public:
    UserController(Pistache::Rest::Router& router);

private:
    void setupRoutes();
    void handleCreateUser(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleUpdateUser(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleGetUsers(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleGetUsersPaginated(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleSearchUsers(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleAddProject(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleUpdateProject(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    

    Pistache::Rest::Router& router;
    UserRepository repository;
}; 