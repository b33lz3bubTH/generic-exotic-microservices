#include "DatabaseUtils.h"
#include <iostream>

namespace DatabaseUtils {

void executeQuery(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw DatabaseException("Database error: " + error);
    }
}

void executeQueryWithCallback(sqlite3* db, const char* sql, 
                            int (*callback)(void*, int, char**, char**), 
                            void* data) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, callback, data, &errMsg) != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw DatabaseException("Database error: " + error);
    }
}

sqlite3_stmt* prepareStatement(sqlite3* db, const char* sql) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw DatabaseException("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }
    return stmt;
}

void bindText(sqlite3_stmt* stmt, int index, const std::string& value) {
    if (sqlite3_bind_text(stmt, index, value.c_str(), -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        throw DatabaseException("Failed to bind text parameter");
    }
}

void bindInt(sqlite3_stmt* stmt, int index, int value) {
    if (sqlite3_bind_int(stmt, index, value) != SQLITE_OK) {
        throw DatabaseException("Failed to bind integer parameter");
    }
}

void finalizeStatement(sqlite3_stmt* stmt) {
    sqlite3_finalize(stmt);
}

} // namespace DatabaseUtils 