#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include "../repositories/ContactUsRepository.h"
#include <vector>
#include <string>

class ContactUsController {
public:
    ContactUsController(Pistache::Rest::Router& router);

private:
    void setupRoutes();
    void handleCreateEntry(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleGetEntriesPaginated(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);

    ContactUsRepository repository;
    Pistache::Rest::Router& router;

    // Key management for security
    std::vector<std::string> keys;
    int currentKeyIndex;
    void loadKeys();
    bool validateKey(const std::string& key, int& keyIndex, int& nextKeyIndex);
}; 