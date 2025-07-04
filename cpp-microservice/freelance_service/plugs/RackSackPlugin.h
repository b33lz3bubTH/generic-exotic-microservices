#pragma once
#include "PluginBase.h"
#include <string>

struct sqlite3; // forward declaration

class RackSackPlugin : public PluginBase {
public:
    void run() override;
    void rollback() override;
    std::string name() const override { return "RackSack"; }
    void setDatabase(sqlite3* db) override { db_ = db; }
private:
    sqlite3* db_ = nullptr;
}; 