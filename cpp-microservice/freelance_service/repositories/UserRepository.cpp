#include "UserRepository.h"
#include <sqlite3.h>
#include <sstream>
#include <random>

UserRepository::UserRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void UserRepository::createTables() {
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id TEXT PRIMARY KEY, "
                      "tenantId TEXT, "
                      "email TEXT, "
                      "password TEXT, "
                      "name TEXT, "
                      "phone TEXT);";
    DatabaseUtils::executeQuery(db, sql);
}

static std::string generate_uuid() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    std::stringstream ss;
    for (int i = 0; i < 32; ++i) ss << std::hex << dis(gen);
    return ss.str();
}

std::string UserRepository::createUser(const UserEntry& user) {
    std::string id = user.id.empty() ? generate_uuid() : user.id;
    const char* sql = "INSERT INTO users (id, tenantId, email, password, name, phone) VALUES (?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, id);
    DatabaseUtils::bindText(stmt, 2, user.tenantId);
    DatabaseUtils::bindText(stmt, 3, user.email);
    DatabaseUtils::bindText(stmt, 4, user.password);
    DatabaseUtils::bindText(stmt, 5, user.name);
    DatabaseUtils::bindText(stmt, 6, user.phone);
    sqlite3_step(stmt);
    DatabaseUtils::finalizeStatement(stmt);
    return id;
}

std::optional<UserEntry> UserRepository::getUserByEmailAndTenant(const std::string& email, const std::string& tenantId) {
    const char* sql = "SELECT id, tenantId, email, password, name, phone FROM users WHERE email = ? AND tenantId = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, email);
    DatabaseUtils::bindText(stmt, 2, tenantId);
    std::optional<UserEntry> user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = UserEntry(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
        );
    }
    DatabaseUtils::finalizeStatement(stmt);
    return user;
}

std::optional<UserEntry> UserRepository::getUserById(const std::string& id) {
    const char* sql = "SELECT id, tenantId, email, password, name, phone FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, id);
    std::optional<UserEntry> user;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        user = UserEntry(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
        );
    }
    DatabaseUtils::finalizeStatement(stmt);
    return user;
} 