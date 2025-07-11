#pragma once
#include "../models/SessionEntry.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <string>
#include <optional>

class SessionRepository {
public:
    SessionRepository();
    std::string createSession(const SessionEntry& session);
    std::optional<SessionEntry> getSessionById(const std::string& id);
    void deleteSession(const std::string& id);
private:
    sqlite3* db;
    void createTables();
}; 