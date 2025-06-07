#include "Database.h"
#include <stdexcept>

std::unique_ptr<Database> Database::instance = nullptr;
std::mutex Database::mutex;

Database& Database::getInstance(const std::string& dbName) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!instance) {
        instance = std::unique_ptr<Database>(new Database(dbName));
    }
    return *instance;
}

Database::Database(const std::string& dbName) {
    if (sqlite3_open(dbName.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db)));
    }
}

Database::~Database() {
    if (db) {
        sqlite3_close(db);
    }
} 