#pragma once
#include <sqlite3.h>
#include <string>

class Database {
public:
    static sqlite3* getInstance(const std::string& db_name = "album_sharing.db");
    static void close();
    static void initialize();
    static bool tableExists(const std::string& tableName);

private:
    static sqlite3* db;
    Database() = default;
    ~Database() = default;
};