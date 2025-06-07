#pragma once
#include "../models/User.h"
#include "../models/Project.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <vector>
#include <string>

class UserRepository {
public:
    UserRepository();
    void createUser(const User& user);
    void updateUser(const User& user);
    std::vector<User> getAllUsers();
    std::vector<User> getUsersPaginated(int skip, int take);
    std::vector<User> searchUsers(const std::string& query);
    int getTotalCount();
    void addProjectToUser(const std::string& email, const Project& project);
    void updateProject(const std::string& email, const Project& project);

private:
    sqlite3* db;
    void createTables();
    User mapUserFromRow(const std::map<std::string, std::string>& row);
    Project mapProjectFromRow(const std::map<std::string, std::string>& row);
    std::vector<Project> getProjectsForUser(int userId);
}; 