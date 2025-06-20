#include "ContactUsRepository.h"
#include <sqlite3.h>
#include <sstream>
#include <iostream>

ContactUsRepository::ContactUsRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void ContactUsRepository::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS contact_us_entries ("
                      "uuid TEXT PRIMARY KEY, "
                      "site TEXT NOT NULL, "
                      "type TEXT NOT NULL, "
                      "message TEXT NOT NULL, "
                      "extras TEXT, "
                      "createdAt TEXT DEFAULT CURRENT_TIMESTAMP"
                      ");";
    DatabaseUtils::executeQuery(db, sql);
}

void ContactUsRepository::createEntry(const ContactUsEntry& entry) {
    const char* sql = "INSERT INTO contact_us_entries (uuid, site, type, message, extras, createdAt) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, entry.uuid);
    DatabaseUtils::bindText(stmt, 2, entry.site);
    DatabaseUtils::bindText(stmt, 3, entry.type);
    DatabaseUtils::bindText(stmt, 4, entry.message);
    DatabaseUtils::bindText(stmt, 5, entry.extras.dump());
    DatabaseUtils::bindText(stmt, 6, entry.createdAt);
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
}

std::vector<ContactUsEntry> ContactUsRepository::getEntriesPaginated(const std::string& site, int skip, int take) {
    std::vector<ContactUsEntry> entries;
    const char* sql = "SELECT * FROM contact_us_entries WHERE site = ? ORDER BY createdAt DESC LIMIT ? OFFSET ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, site);
    DatabaseUtils::bindInt(stmt, 2, take);
    DatabaseUtils::bindInt(stmt, 3, skip);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        ContactUsEntry entry;
        entry.uuid = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        entry.site = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        entry.type = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        entry.message = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        entry.extras = nlohmann::json::parse(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)));
        entry.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        entries.push_back(entry);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return entries;
}

int ContactUsRepository::getTotalCount(const std::string& site) {
    const char* sql = "SELECT COUNT(*) FROM contact_us_entries WHERE site = ?;";
    int count = 0;
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, site);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return count;
} 