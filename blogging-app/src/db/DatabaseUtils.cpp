#include "../include/db/DatabaseUtils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <random>

std::string DatabaseUtils::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

std::string DatabaseUtils::generateUUID() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < 32; i++) {
        int digit = dis(gen);
        ss << std::hex << digit;
    }
    return ss.str();
}

std::string DatabaseUtils::escapeString(const std::string& input) {
    std::string escaped;
    for (char c : input) {
        if (c == '\'') {
            escaped += "''";
        } else {
            escaped += c;
        }
    }
    return escaped;
}

bool DatabaseUtils::executeSQL(sqlite3* db, const std::string& sql, std::string& errorMsg) {
    if (db == nullptr) {
        errorMsg = "Database is null";
        return false;
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        errorMsg = errMsg ? std::string(errMsg) : "Unknown error";
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseUtils::querySQL(sqlite3* db, const std::string& sql,
                            std::vector<std::vector<std::string>>& results,
                            std::string& errorMsg) {
    if (db == nullptr) {
        errorMsg = "Database is null";
        return false;
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        errorMsg = sqlite3_errmsg(db);
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        int colCount = sqlite3_column_count(stmt);
        for (int i = 0; i < colCount; i++) {
            const char* val = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.push_back(val ? std::string(val) : "");
        }
        results.push_back(row);
    }

    sqlite3_finalize(stmt);
    return true;
}

int DatabaseUtils::getLastInsertRowId(sqlite3* db) {
    if (db == nullptr) return -1;
    return static_cast<int>(sqlite3_last_insert_rowid(db));
}
