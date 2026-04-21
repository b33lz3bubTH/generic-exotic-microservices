#include "LogEntry.h"

LogEntry::LogEntry(int id, const std::string& message, const std::string& stackTrace,
                   const nlohmann::json& meta, const std::string& createdAt, const std::string& createdBy)
    : id(id), message(message), stackTrace(stackTrace), meta(meta), createdAt(createdAt), createdBy(createdBy) {}

nlohmann::json LogEntry::to_json() const {
    return {
        {"id", id},
        {"message", message},
        {"stackTrace", stackTrace},
        {"meta", meta},
        {"createdAt", createdAt},
        {"createdBy", createdBy}
    };
}
