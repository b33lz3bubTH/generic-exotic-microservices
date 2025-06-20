#pragma once
#include "../models/ContactUsEntry.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <vector>
#include <string>

class ContactUsRepository {
public:
    ContactUsRepository();
    void createEntry(const ContactUsEntry& entry);
    std::vector<ContactUsEntry> getEntriesPaginated(const std::string& site, int skip, int take);
    int getTotalCount(const std::string& site);
private:
    sqlite3* db;
    void createTables();
}; 