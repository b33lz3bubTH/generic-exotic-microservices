#pragma once
#include "../models/UserEntry.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <string>
#include <optional>

class UserRepository {
public:
    UserRepository();
    std::string createUser(const UserEntry& user);
    std::optional<UserEntry> getUserByEmailAndTenant(const std::string& email, const std::string& tenantId);
    std::optional<UserEntry> getUserById(const std::string& id);
private:
    sqlite3* db;
    void createTables();
}; 