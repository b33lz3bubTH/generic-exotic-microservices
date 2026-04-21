#include "LogRepository.h"
#include <sqlite3.h>
#include <sstream>
#include <iostream>

LogRepository::LogRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void LogRepository::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS logs ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "message TEXT NOT NULL, "
                      "stackTrace TEXT, "
                      "meta TEXT, "
                      "createdAt TEXT DEFAULT CURRENT_TIMESTAMP, "
                      "createdBy TEXT);";
    DatabaseUtils::executeQuery(db, sql);
}

void LogRepository::createLog(const LogEntry& log) {
    const char* sql = "INSERT INTO logs (message, stackTrace, meta, createdBy) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, log.message);
    DatabaseUtils::bindText(stmt, 2, log.stackTrace);
    DatabaseUtils::bindText(stmt, 3, log.meta.dump());
    DatabaseUtils::bindText(stmt, 4, log.createdBy);
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
}

std::vector<LogEntry> LogRepository::searchLogs(const std::string& query) {
    std::vector<LogEntry> logs;
    std::stringstream ss;
    ss << "SELECT * FROM logs WHERE message LIKE ? OR stackTrace LIKE ? OR meta LIKE ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, ss.str().c_str());
    std::string pattern = "%" + query + "%";
    for (int i = 1; i <= 3; ++i) DatabaseUtils::bindText(stmt, i, pattern);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry log;
        log.id = sqlite3_column_int(stmt, 0);
        log.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        log.stackTrace = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        log.meta = nlohmann::json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        log.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        log.createdBy = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        logs.push_back(log);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return logs;
}

std::vector<LogEntry> LogRepository::getLogsPaginated(int skip, int take) {
    std::vector<LogEntry> logs;
    const char* sql = "SELECT * FROM logs ORDER BY createdAt DESC LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindInt(stmt, 1, take);
    DatabaseUtils::bindInt(stmt, 2, skip);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        LogEntry log;
        log.id = sqlite3_column_int(stmt, 0);
        log.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        log.stackTrace = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        log.meta = nlohmann::json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)));
        log.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        log.createdBy = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        logs.push_back(log);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return logs;
}

int LogRepository::getTotalCount() {
    const char* sql = "SELECT COUNT(*) FROM logs;";
    int count = 0;
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return count;
}
