#include "User.h"

User::User(int id, const std::string& name, const std::string& email,
           const std::string& bio, const std::string& title,
           const std::string& location, const std::string& skills,
           const std::string& hourlyRate, const std::string& availability,
           const std::string& experience, const std::string& education,
           const std::string& certifications, const std::string& githubUrl,
           const std::string& linkedinUrl, const std::string& portfolioUrl,
           const std::string& profilePicture)
    : id(id), name(name), email(email), bio(bio), title(title),
      location(location), skills(skills), hourlyRate(hourlyRate),
      availability(availability), experience(experience), education(education),
      certifications(certifications), githubUrl(githubUrl), linkedinUrl(linkedinUrl),
      portfolioUrl(portfolioUrl), profilePicture(profilePicture) {}

nlohmann::json User::to_json() const {
    nlohmann::json projectsArray = nlohmann::json::array();
    for (const auto& project : projects) {
        projectsArray.push_back(project.to_json());
    }

    return {
        {"id", id},
        {"name", name},
        {"email", email},
        {"bio", bio},
        {"title", title},
        {"location", location},
        {"skills", skills},
        {"hourlyRate", hourlyRate},
        {"availability", availability},
        {"experience", experience},
        {"education", education},
        {"certifications", certifications},
        {"githubUrl", githubUrl},
        {"linkedinUrl", linkedinUrl},
        {"portfolioUrl", portfolioUrl},
        {"profilePicture", profilePicture},
        {"projects", projectsArray}
    };
} 