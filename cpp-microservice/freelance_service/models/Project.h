#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class Project {
public:
    int id;
    std::string name;
    std::string description;
    std::string techStack;
    std::string role;
    std::string startDate;
    std::string endDate;
    std::string projectUrl;
    bool isCurrent;

    Project() = default;
    Project(int id, const std::string& name, const std::string& description,
            const std::string& techStack, const std::string& role,
            const std::string& startDate, const std::string& endDate,
            const std::string& projectUrl, bool isCurrent);

    nlohmann::json to_json() const;
}; 