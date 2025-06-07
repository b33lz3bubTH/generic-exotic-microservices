#pragma once

#include <sqlite3.h>
#include <string>
#include <mutex>
#include <memory>

class Database {
public:
    static Database& getInstance(const std::string& dbName);
    sqlite3* getConnection() const { return db; }
    ~Database();

private:
    Database(const std::string& dbName);
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    sqlite3* db;
    static std::unique_ptr<Database> instance;
    static std::mutex mutex;
}; 