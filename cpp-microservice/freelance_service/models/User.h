#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Project.h"

class User {
public:
    int id;
    std::string name;
    std::string email;
    std::string bio;
    std::string title; 
    std::string location;
    std::string skills;
    std::string hourlyRate;
    std::string availability;  
    std::string experience;    
    std::string education;
    std::string certifications;
    std::string githubUrl;
    std::string linkedinUrl;
    std::string portfolioUrl;
    std::string profilePicture;
    std::vector<Project> projects;

    User() = default;
    User(int id, const std::string& name, const std::string& email,
         const std::string& bio, const std::string& title,
         const std::string& location, const std::string& skills,
         const std::string& hourlyRate, const std::string& availability,
         const std::string& experience, const std::string& education,
         const std::string& certifications, const std::string& githubUrl,
         const std::string& linkedinUrl, const std::string& portfolioUrl,
         const std::string& profilePicture);

    nlohmann::json to_json() const;
}; 