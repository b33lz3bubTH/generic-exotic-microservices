#include "RackSackPlugin.h"
#include <iostream>
#include <sqlite3.h>

void RackSackPlugin::run() {
    if (db_) {
        std::cout << "RackSackPlugin run() with DB access" << std::endl;
        // Example: count number of tables in the DB
        const char* sql = "SELECT count(*) FROM sqlite_master WHERE type='table';";
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(db_, sql, -1, &stmt, nullptr);
        if (rc == SQLITE_OK && stmt) {
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int tableCount = sqlite3_column_int(stmt, 0);
                std::cout << "[RackSackPlugin] Number of tables in DB: " << tableCount << std::endl;
            }
            sqlite3_finalize(stmt);
        } else {
            std::cerr << "[RackSackPlugin] Failed to query DB: " << sqlite3_errmsg(db_) << std::endl;
        }
    } else {
        std::cout << "RackSackPlugin run() without DB" << std::endl;
    }
}

void RackSackPlugin::rollback() {
    if (db_) {
        std::cout << "RackSackPlugin rollback() with DB access" << std::endl;
    } else {
        std::cout << "RackSackPlugin rollback() without DB" << std::endl;
    }
} 