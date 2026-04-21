#include "utils/SlugGenerator.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <regex>

const std::vector<std::pair<std::string, std::string>> SlugGenerator::replacements = {
    {"à", "a"}, {"á", "a"}, {"ã", "a"}, {"ä", "a"},
    {"è", "e"}, {"é", "e"}, {"ê", "e"}, {"ë", "e"},
    {"ì", "i"}, {"í", "i"}, {"î", "i"}, {"ï", "i"},
    {"ò", "o"}, {"ó", "o"}, {"ô", "o"}, {"õ", "o"}, {"ö", "o"},
    {"ù", "u"}, {"ú", "u"}, {"û", "u"}, {"ü", "u"},
    {"ý", "y"}, {"ÿ", "y"},
    {"ñ", "n"}, {"ç", "c"},
    {"&", "and"},
    {"@", "at"}
};

std::string SlugGenerator::generate(const std::string& input) {
    std::string result = sanitize(input);
    result = replaceSpaces(result);
    return result;
}

std::string SlugGenerator::sanitize(const std::string& input) {
    std::string result = input;
    
    // Replace accented characters
    for (const auto& [from, to] : replacements) {
        size_t pos = 0;
        while ((pos = result.find(from, pos)) != std::string::npos) {
            result.replace(pos, from.length(), to);
            pos += to.length();
        }
    }
    
    // Convert to lowercase
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    
    // Remove special characters (keep only alphanumeric, spaces, and hyphens)
    result.erase(std::remove_if(result.begin(), result.end(),
                                [](unsigned char c) {
                                    return !std::isalnum(c) && c != ' ' && c != '-';
                                }),
                 result.end());
    
    // Remove leading/trailing whitespace
    auto start = result.find_first_not_of(" \t\n\r");
    auto end = result.find_last_not_of(" \t\n\r");
    result = (start == std::string::npos) ? "" : result.substr(start, end - start + 1);
    
    return result;
}

std::string SlugGenerator::replaceSpaces(const std::string& input) {
    std::string result = input;
    
    // Replace multiple spaces with single space
    std::regex multiple_spaces("  +");
    result = std::regex_replace(result, multiple_spaces, " ");
    
    // Replace spaces with hyphens
    std::replace(result.begin(), result.end(), ' ', '-');
    
    // Remove multiple consecutive hyphens
    std::regex multiple_hyphens("--+");
    result = std::regex_replace(result, multiple_hyphens, "-");
    
    // Remove trailing/leading hyphens
    if (!result.empty() && result.front() == '-') result.erase(0, 1);
    if (!result.empty() && result.back() == '-') result.pop_back();
    
    return result;
}

std::string SlugGenerator::generateUnique(const std::string& input) {
    std::string slug = generate(input);
    
    // Add timestamp for uniqueness
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << slug << "-" << timestamp;
    
    return ss.str();
}
