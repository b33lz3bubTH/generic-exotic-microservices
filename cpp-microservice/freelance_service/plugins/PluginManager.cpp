#include "PluginManager.h"
#include <iostream>

void PluginManager::registerPlugin(std::shared_ptr<PluginBase> plugin) {
    std::lock_guard<std::mutex> lock(mtx);
    plugins.push_back(plugin);
}

void PluginManager::runAll() {
    std::vector<std::shared_ptr<PluginBase>> executed;
    try {
        for (auto& plugin : plugins) {
            std::cout << "Running plugin: " << plugin->name() << std::endl;
            plugin->run();
            executed.push_back(plugin);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in plugin: " << e.what() << ", rolling back..." << std::endl;
        for (auto it = executed.rbegin(); it != executed.rend(); ++it) {
            try {
                (*it)->rollback();
            } catch (const std::exception& ex) {
                std::cerr << "Rollback failed for plugin: " << (*it)->name() << ", error: " << ex.what() << std::endl;
            }
        }
        throw;
    }
}

void PluginManager::runAllParallel() {
    std::vector<std::future<void>> futures;
    std::vector<std::shared_ptr<PluginBase>> executed;
    std::atomic<bool> errorOccurred{false};
    std::exception_ptr firstException = nullptr;
    for (auto& plugin : plugins) {
        futures.push_back(std::async(std::launch::async, [&]() {
            try {
                if (!errorOccurred) {
                    std::cout << "Running plugin: " << plugin->name() << std::endl;
                    plugin->run();
                    executed.push_back(plugin);
                }
            } catch (...) {
                errorOccurred = true;
                if (!firstException) firstException = std::current_exception();
            }
        }));
    }
    for (auto& f : futures) {
        try { f.get(); } catch (...) {}
    }
    if (errorOccurred) {
        std::cerr << "Error in parallel plugin execution, rolling back all..." << std::endl;
        for (auto& plugin : executed) {
            try {
                plugin->rollback();
            } catch (const std::exception& ex) {
                std::cerr << "Rollback failed for plugin: " << plugin->name() << ", error: " << ex.what() << std::endl;
            }
        }
        if (firstException) std::rethrow_exception(firstException);
    }
}

void PluginManager::setDatabaseForAll(sqlite3* db) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& plugin : plugins) {
        plugin->setDatabase(db);
    }
} 