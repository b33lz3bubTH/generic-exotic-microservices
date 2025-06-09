#pragma once
#include <pistache/http.h>
#include <pistache/router.h>
#include "../repositories/VaultRepository.h"
#include <string>

class VaultController {
public:
    VaultController(Pistache::Rest::Router& router);
private:
    void handleCreateVault(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    void handleGetVault(const Pistache::Rest::Request&, Pistache::Http::ResponseWriter);
    VaultRepository repository;
};
