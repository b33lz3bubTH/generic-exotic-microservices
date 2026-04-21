#pragma once

#include <string>
#include <vector>

class SlugGenerator {
public:
    // Generate URL-safe slug from string
    static std::string generate(const std::string& input);
    
    // Remove special characters and convert to lowercase
    static std::string sanitize(const std::string& input);
    
    // Replace spaces with hyphens
    static std::string replaceSpaces(const std::string& input);
    
    // Generate slug with timestamp for uniqueness
    static std::string generateUnique(const std::string& input);

private:
    static const std::vector<std::pair<std::string, std::string>> replacements;
};
