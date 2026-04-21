#pragma once
#include "../models/VaultEntry.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <string>
#include <optional>

class VaultRepository {
public:
    VaultRepository();
    std::string createVaultEntry(const std::string& encryptedData);
    std::optional<VaultEntry> getVaultEntry(const std::string& uuid);
private:
    sqlite3* db;
    void createTables();
};
