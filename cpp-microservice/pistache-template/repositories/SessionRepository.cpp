#include "SessionRepository.h"
#include <sqlite3.h>
#include <sstream>
#include <random>

SessionRepository::SessionRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void SessionRepository::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS sessions ("
                      "id TEXT PRIMARY KEY, "
                      "userId TEXT, "
                      "expiresAt TEXT, "
                      "userDetails TEXT);";
    DatabaseUtils::executeQuery(db, sql);
}

static std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) ss << std::hex << dis(gen);
    return ss.str();
}

std::string SessionRepository::createSession(const SessionEntry& session) {
    std::string id = session.id.empty() ? generate_uuid() : session.id;
    const char* sql = "INSERT INTO sessions (id, userId, expiresAt, userDetails) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, id);
    DatabaseUtils::bindText(stmt, 2, session.userId);
    DatabaseUtils::bindText(stmt, 3, session.expiresAt);
    DatabaseUtils::bindText(stmt, 4, session.userDetails.dump());
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
    return id;
}

std::optional<SessionEntry> SessionRepository::getSessionById(const std::string& id) {
    const char* sql = "SELECT id, userId, expiresAt, userDetails FROM sessions WHERE id = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, id);
    std::optional<SessionEntry> session;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        session = SessionEntry(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            nlohmann::json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))
        );
    }
    DatabaseUtils::finalizeStatement(stmt);
    return session;
}

void SessionRepository::deleteSession(const std::string& id) {
    const char* sql = "DELETE FROM sessions WHERE id = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, id);
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
} 