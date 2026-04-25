#pragma once
#include <string>
#include <vector>
#include <sqlite3.h>

class DatabaseUtils {
public:
    static std::string getCurrentTimestamp();
    static std::string generateUUID();
    static std::string escapeString(const std::string& input);
    static bool executeSQL(sqlite3* db, const std::string& sql, std::string& errorMsg);
    static bool querySQL(sqlite3* db, const std::string& sql, 
                         std::vector<std::vector<std::string>>& results, 
                         std::string& errorMsg);
    static int getLastInsertRowId(sqlite3* db);
};
