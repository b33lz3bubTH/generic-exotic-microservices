#pragma once
#include <sqlite3.h>
#include <string>

class Database {
public:
    static sqlite3* getInstance(const std::string& db_name);
    static void close();

private:
    static sqlite3* db;
    Database() = default;
}; 