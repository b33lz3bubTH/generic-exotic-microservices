#include "UserController.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <random>

using json = nlohmann::json;
using namespace Pistache;

static std::string generate_session_id() {
    static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static thread_local std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    std::string s;
    for (int i = 0; i < 32; ++i) s += alphanum[dis(gen)];
    return s;
}

static std::string get_expiry(int hours = 24) {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto expiry = now + hours * 1h;
    std::time_t expiry_time = system_clock::to_time_t(expiry);
    char buf[32];
    std::strftime(buf, sizeof(buf), "%FT%TZ", std::gmtime(&expiry_time));
    return buf;
}

UserController::UserController(Rest::Router& r) : router(r) {
    setupRoutes();
}

void UserController::setupRoutes() {
    Rest::Routes::Post(router, "/users/signup", Rest::Routes::bind(&UserController::handleSignup, this));
    Rest::Routes::Post(router, "/users/login", Rest::Routes::bind(&UserController::handleLogin, this));
    Rest::Routes::Get(router, "/users/auth/me", Rest::Routes::bind(&UserController::handleAuthMe, this));
    Rest::Routes::Get(router, "/users/session/check", Rest::Routes::bind(&UserController::handleSessionCheck, this));
}

void UserController::handleSignup(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto data = json::parse(request.body());
        std::string tenantId = data.value("tenantId", "");
        UserEntry user{"", tenantId, data.value("email", ""), data.value("password", ""), data.value("name", ""), data.value("phone", "")};
        std::string userId = userRepository.createUser(user);
        user.id = userId;
        // Create session
        std::string sessionId = generate_session_id();
        std::string expiresAt = get_expiry();
        SessionEntry session{sessionId, userId, expiresAt, user.to_json()};
        sessionRepository.createSession(session);
        response.send(Http::Code::Created, json({{"sessionId", sessionId}, {"expiresAt", expiresAt}}).dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", e.what()}}).dump(), MIME(Application, Json));
    }
}

void UserController::handleLogin(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto data = json::parse(request.body());
        std::string tenantId = data.value("tenantId", "");
        std::string email = data.value("email", "");
        std::string password = data.value("password", "");
        auto userOpt = userRepository.getUserByEmailAndTenant(email, tenantId);
        if (!userOpt || userOpt->password != password) {
            response.send(Http::Code::Unauthorized, json({{"error", "Invalid credentials"}}).dump(), MIME(Application, Json));
            return;
        }
        // Create session
        std::string sessionId = generate_session_id();
        std::string expiresAt = get_expiry();
        SessionEntry session{sessionId, userOpt->id, expiresAt, userOpt->to_json()};
        sessionRepository.createSession(session);
        response.send(Http::Code::Ok, json({{"sessionId", sessionId}, {"expiresAt", expiresAt}}).dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        response.send(Http::Code::Bad_Request, json({{"error", e.what()}}).dump(), MIME(Application, Json));
    }
}

void UserController::handleAuthMe(const Rest::Request& request, Http::ResponseWriter response) {
    auto sessionId = request.query().get("sessionId");
    if (!sessionId) {
        response.send(Http::Code::Bad_Request, json({{"error", "Missing sessionId"}}).dump(), MIME(Application, Json));
        return;
    }
    auto sessionOpt = sessionRepository.getSessionById(*sessionId);
    if (!sessionOpt) {
        response.send(Http::Code::Unauthorized, json({{"error", "Invalid session"}}).dump(), MIME(Application, Json));
        return;
    }
    response.send(Http::Code::Ok, sessionOpt->userDetails.dump(), MIME(Application, Json));
}

void UserController::handleSessionCheck(const Rest::Request& request, Http::ResponseWriter response) {
    auto sessionId = request.query().get("sessionId");
    if (!sessionId) {
        response.send(Http::Code::Bad_Request, json({{"error", "Missing sessionId"}}).dump(), MIME(Application, Json));
        return;
    }
    auto sessionOpt = sessionRepository.getSessionById(*sessionId);
    if (!sessionOpt) {
        response.send(Http::Code::Unauthorized, json({{"ok", 401}}).dump(), MIME(Application, Json));
        return;
    }
    // Optionally check expiry here
    response.send(Http::Code::Ok, json({{"ok", 200}}).dump(), MIME(Application, Json));
} 