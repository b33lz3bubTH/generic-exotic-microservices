#include "UserRepository.h"
#include "../db/Database.h"
#include "../db/DatabaseUtils.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>

UserRepository::UserRepository() : db(Database::getInstance(DatabaseUtils::DB_NAME)) {
    createTables();
}

void UserRepository::createTables() {
    // Create users table with new fields
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "name TEXT NOT NULL, "
                      "email TEXT, "
                      "bio TEXT, "
                      "title TEXT, "
                      "location TEXT, "
                      "skills TEXT, "
                      "hourlyRate TEXT, "
                      "availability TEXT, "
                      "experience TEXT, "
                      "education TEXT, "
                      "certifications TEXT, "
                      "githubUrl TEXT, "
                      "linkedinUrl TEXT, "
                      "portfolioUrl TEXT, "
                      "profilePicture TEXT);";

    // Create projects table
    const char* projects_sql = "CREATE TABLE IF NOT EXISTS projects ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "user_id INTEGER, "
                              "name TEXT NOT NULL, "
                              "description TEXT, "
                              "techStack TEXT, "
                              "role TEXT, "
                              "startDate TEXT, "
                              "endDate TEXT, "
                              "projectUrl TEXT, "
                              "isCurrent INTEGER, "
                              "FOREIGN KEY(user_id) REFERENCES users(id));";

    try {
        DatabaseUtils::executeQuery(db, sql);
        DatabaseUtils::executeQuery(db, projects_sql);
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << e.what() << std::endl;
    }
}

void UserRepository::createUser(const User& user) {
    const char* sql = "INSERT INTO users (name, email, bio, title, location, skills, "
                      "hourlyRate, availability, experience, education, certifications, "
                      "githubUrl, linkedinUrl, portfolioUrl, profilePicture) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(
            db, sql);
        
        DatabaseUtils::bindText(stmt, 1, user.name);
        DatabaseUtils::bindText(stmt, 2, user.email);
        DatabaseUtils::bindText(stmt, 3, user.bio);
        DatabaseUtils::bindText(stmt, 4, user.title);
        DatabaseUtils::bindText(stmt, 5, user.location);
        DatabaseUtils::bindText(stmt, 6, user.skills);
        DatabaseUtils::bindText(stmt, 7, user.hourlyRate);
        DatabaseUtils::bindText(stmt, 8, user.availability);
        DatabaseUtils::bindText(stmt, 9, user.experience);
        DatabaseUtils::bindText(stmt, 10, user.education);
        DatabaseUtils::bindText(stmt, 11, user.certifications);
        DatabaseUtils::bindText(stmt, 12, user.githubUrl);
        DatabaseUtils::bindText(stmt, 13, user.linkedinUrl);
        DatabaseUtils::bindText(stmt, 14, user.portfolioUrl);
        DatabaseUtils::bindText(stmt, 15, user.profilePicture);
        
        sqlite3_step(stmt);
        int userId = sqlite3_last_insert_rowid(db);
        DatabaseUtils::finalizeStatement(stmt);

        // Insert projects if any
        if (!user.projects.empty()) {
            const char* project_sql = "INSERT INTO projects (user_id, name, description, "
                                    "techStack, role, startDate, endDate, projectUrl, isCurrent) "
                                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
            
            for (const auto& project : user.projects) {
                sqlite3_stmt* project_stmt = DatabaseUtils::prepareStatement(
                    db, project_sql);
                
                DatabaseUtils::bindInt(project_stmt, 1, userId);
                DatabaseUtils::bindText(project_stmt, 2, project.name);
                DatabaseUtils::bindText(project_stmt, 3, project.description);
                DatabaseUtils::bindText(project_stmt, 4, project.techStack);
                DatabaseUtils::bindText(project_stmt, 5, project.role);
                DatabaseUtils::bindText(project_stmt, 6, project.startDate);
                DatabaseUtils::bindText(project_stmt, 7, project.endDate);
                DatabaseUtils::bindText(project_stmt, 8, project.projectUrl);
                DatabaseUtils::bindInt(project_stmt, 9, project.isCurrent ? 1 : 0);
                
                sqlite3_step(project_stmt);
                DatabaseUtils::finalizeStatement(project_stmt);
            }
        }
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << e.what() << std::endl;
    }
}

std::vector<User> UserRepository::getAllUsers() {
    const char* sql = "SELECT * FROM users;";
    std::vector<User> users;

    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(
            db, sql);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            User user;
            user.id = sqlite3_column_int(stmt, 0);
            user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            user.bio = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            user.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            user.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            user.skills = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            user.hourlyRate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            user.availability = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            user.experience = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            user.education = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            user.certifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
            user.githubUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
            user.linkedinUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
            user.portfolioUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
            user.profilePicture = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));

            // Get projects for this user
            const char* projects_sql = "SELECT * FROM projects WHERE user_id = ?;";
            sqlite3_stmt* projects_stmt = DatabaseUtils::prepareStatement(
                db, projects_sql);
            DatabaseUtils::bindInt(projects_stmt, 1, user.id);

            while (sqlite3_step(projects_stmt) == SQLITE_ROW) {
                Project project;
                project.id = sqlite3_column_int(projects_stmt, 0);
                project.name = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 2));
                project.description = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 3));
                project.techStack = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 4));
                project.role = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 5));
                project.startDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 6));
                project.endDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 7));
                project.projectUrl = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 8));
                project.isCurrent = sqlite3_column_int(projects_stmt, 9) == 1;
                user.projects.push_back(project);
            }
            DatabaseUtils::finalizeStatement(projects_stmt);
            users.push_back(user);
        }
        DatabaseUtils::finalizeStatement(stmt);
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << e.what() << std::endl;
    }
    return users;
}

std::vector<User> UserRepository::getUsersPaginated(int skip, int take) {
    std::vector<User> users;
    const char* sql = "SELECT * FROM users LIMIT ? OFFSET ?;";

    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);

        DatabaseUtils::bindInt(stmt, 1, take);
        DatabaseUtils::bindInt(stmt, 2, skip);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            User user;
            user.id = sqlite3_column_int(stmt, 0);
            user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            user.bio = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            user.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            user.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            user.skills = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            user.hourlyRate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            user.availability = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            user.experience = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            user.education = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            user.certifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
            user.githubUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
            user.linkedinUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
            user.portfolioUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
            user.profilePicture = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));

            // Get projects for this user
            const char* projects_sql = "SELECT * FROM projects WHERE user_id = ?;";
            sqlite3_stmt* projects_stmt = DatabaseUtils::prepareStatement(db, projects_sql);
            DatabaseUtils::bindInt(projects_stmt, 1, user.id);

            while (sqlite3_step(projects_stmt) == SQLITE_ROW) {
                Project project;
                project.id = sqlite3_column_int(projects_stmt, 0);
                project.name = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 2));
                project.description = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 3));
                project.techStack = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 4));
                project.role = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 5));
                project.startDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 6));
                project.endDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 7));
                project.projectUrl = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 8));
                project.isCurrent = sqlite3_column_int(projects_stmt, 9) == 1;
                user.projects.push_back(project);
            }
            DatabaseUtils::finalizeStatement(projects_stmt);
            users.push_back(user);
        }

        DatabaseUtils::finalizeStatement(stmt);
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << "[DB Error] " << e.what() << std::endl;
    }

    return users;
}

int UserRepository::getTotalCount() {
    const char* sql = "SELECT COUNT(*) FROM users;";
    int count = 0;

    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }

        DatabaseUtils::finalizeStatement(stmt);
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << "[DB Error] " << e.what() << std::endl;
    }

    return count;
}

void UserRepository::updateUser(const User& user) {
    const char* sql = "UPDATE users SET "
                      "bio = ?, title = ?, location = ?, skills = ?, "
                      "hourlyRate = ?, availability = ?, experience = ?, "
                      "education = ?, certifications = ?, githubUrl = ?, "
                      "linkedinUrl = ?, portfolioUrl = ?, profilePicture = ? "
                      "WHERE id = ?;";

    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(
            db, sql);
        
        DatabaseUtils::bindText(stmt, 1, user.bio);
        DatabaseUtils::bindText(stmt, 2, user.title);
        DatabaseUtils::bindText(stmt, 3, user.location);
        DatabaseUtils::bindText(stmt, 4, user.skills);
        DatabaseUtils::bindText(stmt, 5, user.hourlyRate);
        DatabaseUtils::bindText(stmt, 6, user.availability);
        DatabaseUtils::bindText(stmt, 7, user.experience);
        DatabaseUtils::bindText(stmt, 8, user.education);
        DatabaseUtils::bindText(stmt, 9, user.certifications);
        DatabaseUtils::bindText(stmt, 10, user.githubUrl);
        DatabaseUtils::bindText(stmt, 11, user.linkedinUrl);
        DatabaseUtils::bindText(stmt, 12, user.portfolioUrl);
        DatabaseUtils::bindText(stmt, 13, user.profilePicture);
        DatabaseUtils::bindInt(stmt, 14, user.id);
        
        sqlite3_step(stmt);
        DatabaseUtils::finalizeStatement(stmt);

        // Delete existing projects
        const char* delete_sql = "DELETE FROM projects WHERE user_id = ?;";
        sqlite3_stmt* delete_stmt = DatabaseUtils::prepareStatement(
            db, delete_sql);
        DatabaseUtils::bindInt(delete_stmt, 1, user.id);
        sqlite3_step(delete_stmt);
        DatabaseUtils::finalizeStatement(delete_stmt);

        // Insert updated projects
        if (!user.projects.empty()) {
            const char* project_sql = "INSERT INTO projects (user_id, name, description, "
                                    "techStack, role, startDate, endDate, projectUrl, isCurrent) "
                                    "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
            
            for (const auto& project : user.projects) {
                sqlite3_stmt* project_stmt = DatabaseUtils::prepareStatement(
                    db, project_sql);
                
                DatabaseUtils::bindInt(project_stmt, 1, user.id);
                DatabaseUtils::bindText(project_stmt, 2, project.name);
                DatabaseUtils::bindText(project_stmt, 3, project.description);
                DatabaseUtils::bindText(project_stmt, 4, project.techStack);
                DatabaseUtils::bindText(project_stmt, 5, project.role);
                DatabaseUtils::bindText(project_stmt, 6, project.startDate);
                DatabaseUtils::bindText(project_stmt, 7, project.endDate);
                DatabaseUtils::bindText(project_stmt, 8, project.projectUrl);
                DatabaseUtils::bindInt(project_stmt, 9, project.isCurrent ? 1 : 0);
                
                sqlite3_step(project_stmt);
                DatabaseUtils::finalizeStatement(project_stmt);
            }
        }
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
}

std::vector<User> UserRepository::searchUsers(const std::string& query) {
    std::vector<User> users;
    std::stringstream ss;
    ss << "SELECT * FROM users WHERE "
       << "name LIKE ? OR "
       << "bio LIKE ? OR "
       << "title LIKE ? OR "
       << "location LIKE ? OR "
       << "skills LIKE ? OR "
       << "education LIKE ? OR "
       << "certifications LIKE ?;";

    try {
        sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(
            db, ss.str().c_str());

        std::string searchPattern = "%" + query + "%";
        for (int i = 1; i <= 7; i++) {
            DatabaseUtils::bindText(stmt, i, searchPattern);
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            User user;
            user.id = sqlite3_column_int(stmt, 0);
            user.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            user.bio = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            user.title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            user.location = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            user.skills = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            user.hourlyRate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            user.availability = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            user.experience = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9));
            user.education = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10));
            user.certifications = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
            user.githubUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
            user.linkedinUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
            user.portfolioUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 14));
            user.profilePicture = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 15));

            // Get projects for this user
            const char* projects_sql = "SELECT * FROM projects WHERE user_id = ?;";
            sqlite3_stmt* projects_stmt = DatabaseUtils::prepareStatement(
                db, projects_sql);
            DatabaseUtils::bindInt(projects_stmt, 1, user.id);

            while (sqlite3_step(projects_stmt) == SQLITE_ROW) {
                Project project;
                project.id = sqlite3_column_int(projects_stmt, 0);
                project.name = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 2));
                project.description = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 3));
                project.techStack = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 4));
                project.role = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 5));
                project.startDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 6));
                project.endDate = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 7));
                project.projectUrl = reinterpret_cast<const char*>(sqlite3_column_text(projects_stmt, 8));
                project.isCurrent = sqlite3_column_int(projects_stmt, 9) == 1;
                user.projects.push_back(project);
            }
            DatabaseUtils::finalizeStatement(projects_stmt);
            users.push_back(user);
        }
        DatabaseUtils::finalizeStatement(stmt);
    } catch (const DatabaseUtils::DatabaseException& e) {
        std::cerr << e.what() << std::endl;
    }
    return users;
}

void UserRepository::addProjectToUser(const std::string& email, const Project& project) {
    // First get the user ID from email
    const char* sql = "SELECT id FROM users WHERE email = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, email);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
    }
    DatabaseUtils::finalizeStatement(stmt);

    if (userId == -1) {
        throw std::runtime_error("User not found with email: " + email);
    }

    // Insert the project
    const char* insert_sql = "INSERT INTO projects (user_id, name, description, techStack, role, startDate, endDate, projectUrl, isCurrent) "
                           "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* insert_stmt = DatabaseUtils::prepareStatement(db, insert_sql);
    
    DatabaseUtils::bindInt(insert_stmt, 1, userId);
    DatabaseUtils::bindText(insert_stmt, 2, project.name);
    DatabaseUtils::bindText(insert_stmt, 3, project.description);
    DatabaseUtils::bindText(insert_stmt, 4, project.techStack);
    DatabaseUtils::bindText(insert_stmt, 5, project.role);
    DatabaseUtils::bindText(insert_stmt, 6, project.startDate);
    DatabaseUtils::bindText(insert_stmt, 7, project.endDate);
    DatabaseUtils::bindText(insert_stmt, 8, project.projectUrl);
    DatabaseUtils::bindInt(insert_stmt, 9, project.isCurrent ? 1 : 0);

    sqlite3_step(insert_stmt);
    DatabaseUtils::finalizeStatement(insert_stmt);
}

void UserRepository::updateProject(const std::string& email, const Project& project) {
    // First get the user ID from email
    const char* sql = "SELECT id FROM users WHERE email = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindText(stmt, 1, email);

    int userId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        userId = sqlite3_column_int(stmt, 0);
    }
    DatabaseUtils::finalizeStatement(stmt);

    if (userId == -1) {
        throw std::runtime_error("User not found with email: " + email);
    }

    // Verify the project belongs to the user
    const char* verify_sql = "SELECT id FROM projects WHERE id = ? AND user_id = ?;";
    sqlite3_stmt* verify_stmt = DatabaseUtils::prepareStatement(db, verify_sql);
    DatabaseUtils::bindInt(verify_stmt, 1, project.id);
    DatabaseUtils::bindInt(verify_stmt, 2, userId);

    bool projectExists = sqlite3_step(verify_stmt) == SQLITE_ROW;
    DatabaseUtils::finalizeStatement(verify_stmt);

    if (!projectExists) {
        throw std::runtime_error("Project not found or does not belong to user");
    }

    // Update the project
    const char* update_sql = "UPDATE projects SET "
                           "name = ?, description = ?, techStack = ?, role = ?, "
                           "startDate = ?, endDate = ?, projectUrl = ?, isCurrent = ? "
                           "WHERE id = ? AND user_id = ?;";
    sqlite3_stmt* update_stmt = DatabaseUtils::prepareStatement(db, update_sql);
    
    DatabaseUtils::bindText(update_stmt, 1, project.name);
    DatabaseUtils::bindText(update_stmt, 2, project.description);
    DatabaseUtils::bindText(update_stmt, 3, project.techStack);
    DatabaseUtils::bindText(update_stmt, 4, project.role);
    DatabaseUtils::bindText(update_stmt, 5, project.startDate);
    DatabaseUtils::bindText(update_stmt, 6, project.endDate);
    DatabaseUtils::bindText(update_stmt, 7, project.projectUrl);
    DatabaseUtils::bindInt(update_stmt, 8, project.isCurrent ? 1 : 0);
    DatabaseUtils::bindInt(update_stmt, 9, project.id);
    DatabaseUtils::bindInt(update_stmt, 10, userId);

    sqlite3_step(update_stmt);
    DatabaseUtils::finalizeStatement(update_stmt);
}

Project UserRepository::mapProjectFromRow(const std::map<std::string, std::string>& row) {
    Project project;
    project.id = std::stoi(row.at("id"));
    project.name = row.at("name");
    project.description = row.at("description");
    project.techStack = row.at("tech_stack");
    project.role = row.at("role");
    project.startDate = row.at("start_date");
    project.endDate = row.at("end_date");
    project.projectUrl = row.at("project_url");
    project.isCurrent = row.at("is_current") == "1";
    return project;
}

std::vector<Project> UserRepository::getProjectsForUser(int userId) {
    std::vector<Project> projects;
    const char* sql = "SELECT * FROM projects WHERE user_id = ?;";
    sqlite3_stmt* stmt = DatabaseUtils::prepareStatement(db, sql);
    DatabaseUtils::bindInt(stmt, 1, userId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Project project;
        project.id = sqlite3_column_int(stmt, 0);
        project.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        project.description = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        project.techStack = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        project.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        project.startDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
        project.endDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
        project.projectUrl = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        project.isCurrent = sqlite3_column_int(stmt, 9) == 1;
        projects.push_back(project);
    }
    DatabaseUtils::finalizeStatement(stmt);
    return projects;
}
