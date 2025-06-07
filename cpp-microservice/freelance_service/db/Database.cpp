#include "Database.h"
#include <stdexcept>

sqlite3* Database::db = nullptr;

sqlite3* Database::getInstance(const std::string& db_name) {
    if (!db) {
        if (sqlite3_open(db_name.c_str(), &db)) {
            throw std::runtime_error("Can't open DB: " + std::string(sqlite3_errmsg(db)));
        }
    }
    return db;
}

void Database::close() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
} 