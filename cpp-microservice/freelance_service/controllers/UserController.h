#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include "../repositories/UserRepository.h"
#include "../repositories/SessionRepository.h"

class UserController {
public:
    UserController(Pistache::Rest::Router& router);
private:
    void setupRoutes();
    void handleSignup(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleLogin(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleAuthMe(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleSessionCheck(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    UserRepository userRepository;
    SessionRepository sessionRepository;
    Pistache::Rest::Router& router;
}; 