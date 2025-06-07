#include "Project.h"

Project::Project(int id, const std::string& name, const std::string& description,
                const std::string& techStack, const std::string& role,
                const std::string& startDate, const std::string& endDate,
                const std::string& projectUrl, bool isCurrent)
    : id(id), name(name), description(description), techStack(techStack),
      role(role), startDate(startDate), endDate(endDate),
      projectUrl(projectUrl), isCurrent(isCurrent) {}

nlohmann::json Project::to_json() const {
    return {
        {"id", id},
        {"name", name},
        {"description", description},
        {"techStack", techStack},
        {"role", role},
        {"startDate", startDate},
        {"endDate", endDate},
        {"projectUrl", projectUrl},
        {"isCurrent", isCurrent}
    };
} 