#pragma once
#include <string>
#include <sqlite3.h>

class PluginBase {
public:
    virtual ~PluginBase() = default;
    // The main logic of the plugin
    virtual void run() = 0;
    // Rollback logic in case of error
    virtual void rollback() = 0;
    // Optional: plugin name for logging/debugging
    virtual std::string name() const = 0;
    // Optional: inject database pointer
    virtual void setDatabase(sqlite3*) {}
}; 