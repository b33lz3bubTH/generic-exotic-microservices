#pragma once
#include "../models/LogEntry.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <vector>
#include <string>

class LogRepository {
public:
    LogRepository();
    void createLog(const LogEntry& log);
    std::vector<LogEntry> searchLogs(const std::string& query);
    std::vector<LogEntry> getLogsPaginated(int skip, int take);
    int getTotalCount();

private:
    sqlite3* db;
    void createTables();
};
