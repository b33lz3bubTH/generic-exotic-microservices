#include "VaultRepository.h"
#include <sqlite3.h>
#include <sstream>
#include <iostream>
#include <random>
#include <iomanip>

VaultRepository::VaultRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void VaultRepository::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS vault ("
                      "uuid TEXT PRIMARY KEY, "
                      "encryptedData TEXT NOT NULL, "
                      "createdAt TEXT DEFAULT CURRENT_TIMESTAMP);";
    DatabaseUtils::executeQuery(db, sql);
}

// Simple random hex string as UUID
static std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) ss << std::hex << dis(gen);
    return ss.str();
}

std::string VaultRepository::createVaultEntry(const std::string& encryptedData) {
    std::string uuid = generate_uuid();
    const char* sql = "INSERT INTO vault (uuid, encryptedData) VALUES (?, ?);";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, uuid);
    DatabaseUtils::bindText(stmt, 2, encryptedData);
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
    return uuid;
}

std::optional<VaultEntry> VaultRepository::getVaultEntry(const std::string& uuid) {
    const char* sql = "SELECT uuid, encryptedData, createdAt FROM vault WHERE uuid = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, uuid);
    std::optional<VaultEntry> entry;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        entry = VaultEntry(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
        );
    }
    DatabaseUtils::finalizeStatement(stmt);
    return entry;
}
