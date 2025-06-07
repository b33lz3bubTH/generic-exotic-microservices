#pragma once
#include <sqlite3.h>
#include <string>
#include <stdexcept>

namespace DatabaseUtils {
    // Constants
    constexpr const char* DB_NAME = "freelancers.db";
    
    // Error handling
    class DatabaseException : public std::runtime_error {
    public:
        explicit DatabaseException(const std::string& message) : std::runtime_error(message) {}
    };

    // Utility functions
    void executeQuery(sqlite3* db, const char* sql);
    void executeQueryWithCallback(sqlite3* db, const char* sql, 
                                int (*callback)(void*, int, char**, char**), 
                                void* data);
    
    // Prepared statement utilities
    sqlite3_stmt* prepareStatement(sqlite3* db, const char* sql);
    void bindText(sqlite3_stmt* stmt, int index, const std::string& value);
    void bindInt(sqlite3_stmt* stmt, int index, int value);
    void finalizeStatement(sqlite3_stmt* stmt);
} 