#pragma once
#include <string>
#include <nlohmann/json.hpp>

class LogEntry {
public:
    int id;
    std::string message;
    std::string stackTrace;
    nlohmann::json meta;
    std::string createdAt;
    std::string createdBy;

    LogEntry() = default;
    LogEntry(int id, const std::string& message, const std::string& stackTrace,
             const nlohmann::json& meta, const std::string& createdAt, const std::string& createdBy);

    nlohmann::json to_json() const;
};
