#pragma once
#include "PluginBase.h"
#include <vector>
#include <memory>
#include <string>
#include <mutex>
#include <future>
#include <sqlite3.h>

class PluginManager {
public:
    void registerPlugin(std::shared_ptr<PluginBase> plugin);
    // Run all plugins sequentially, rollback on error
    void runAll();
    // Run all plugins in parallel, rollback all if any fails
    void runAllParallel();
    // Inject database pointer to all plugins
    void setDatabaseForAll(sqlite3* db);
private:
    std::vector<std::shared_ptr<PluginBase>> plugins;
    std::mutex mtx;
}; 