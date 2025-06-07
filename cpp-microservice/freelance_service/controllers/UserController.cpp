#include "UserController.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace Pistache;

UserController::UserController(Rest::Router& r) : router(r) {
    setupRoutes();
}

void UserController::setupRoutes() {
    Rest::Routes::Post(router, "/users", Rest::Routes::bind(&UserController::handleCreateUser, this));
    Rest::Routes::Put(router, "/users/:id", Rest::Routes::bind(&UserController::handleUpdateUser, this));
    Rest::Routes::Get(router, "/users", Rest::Routes::bind(&UserController::handleGetUsers, this));
    Rest::Routes::Get(router, "/users/list", Rest::Routes::bind(&UserController::handleGetUsersPaginated, this));
    Rest::Routes::Get(router, "/users/search", Rest::Routes::bind(&UserController::handleSearchUsers, this));
    Rest::Routes::Post(router, "/users/:email/projects", Rest::Routes::bind(&UserController::handleAddProject, this));
    Rest::Routes::Put(router, "/users/:email/projects/:projectId", Rest::Routes::bind(&UserController::handleUpdateProject, this));
}

void UserController::handleCreateUser(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto body = request.body();
        auto data = json::parse(body);

        // Create user with basic info
        User user{0, 
            data["name"].get<std::string>(),
            data["email"].get<std::string>(),
            data["bio"].get<std::string>(),
            data["title"].get<std::string>(),
            data["location"].get<std::string>(),
            data["skills"].get<std::string>(),
            data["hourlyRate"].get<std::string>(),
            data["availability"].get<std::string>(),
            data["experience"].get<std::string>(),
            data["education"].get<std::string>(),
            data["certifications"].get<std::string>(),
            data["githubUrl"].get<std::string>(),
            data["linkedinUrl"].get<std::string>(),
            data["portfolioUrl"].get<std::string>(),
            data["profilePicture"].get<std::string>()
        };

        // Add projects if present
        if (data.contains("projects") && data["projects"].is_array()) {
            for (const auto& projectData : data["projects"]) {
                Project project{
                    0,
                    projectData["name"].get<std::string>(),
                    projectData["description"].get<std::string>(),
                    projectData["techStack"].get<std::string>(),
                    projectData["role"].get<std::string>(),
                    projectData["startDate"].get<std::string>(),
                    projectData["endDate"].get<std::string>(),
                    projectData["projectUrl"].get<std::string>(),
                    projectData["isCurrent"].get<bool>()
                };
                user.projects.push_back(project);
            }
        }

        repository.createUser(user);

        json responseJson = {
            {"message", "User created successfully"},
            {"status", "success"}
        };
        response.send(Http::Code::Created, responseJson.dump(), MIME(Application, Json));
    } catch (const json::exception& e) {
        json error = {
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
        response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to create user"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleGetUsers(const Rest::Request&, Http::ResponseWriter response) {
    try {
        auto users = repository.getAllUsers();
        json usersArray = json::array();

        for (const auto& user : users) {
            usersArray.push_back(user.to_json());
        }

        json responseJson = {
            {"system_message", "Current Users"},
            {"users", usersArray}
        };

        response.send(Http::Code::Ok, responseJson.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to fetch users"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleGetUsersPaginated(const Rest::Request& request, Http::ResponseWriter response) {
    int take = 10;  // default value
    int skip = 0;   // default value

    // Handle take parameter
    if (auto takeOpt = request.query().get("take")) {
        try {
            take = std::stoi(*takeOpt);
        } catch (const std::exception& e) {
            json error = {
                {"error", "Invalid 'take' parameter"},
                {"details", e.what()}
            };
            response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
            return;
        }
    }

    // Handle skip parameter
    if (auto skipOpt = request.query().get("skip")) {
        try {
            skip = std::stoi(*skipOpt);
        } catch (const std::exception& e) {
            json error = {
                {"error", "Invalid 'skip' parameter"},
                {"details", e.what()}
            };
            response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
            return;
        }
    }

    try {
        // Fetch paginated users and total count
        auto users = repository.getUsersPaginated(skip, take);
        int totalCount = repository.getTotalCount();

        json usersArray = json::array();
        for (const auto& user : users) {
            usersArray.push_back(user.to_json());
        }

        json responseJson = {
            {"system_message", "Paginated Users"},
            {"skip", skip},
            {"take", take},
            {"total", totalCount},
            {"users", usersArray}
        };

        response.send(Http::Code::Ok, responseJson.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to fetch paginated users"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleUpdateUser(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto id = request.param(":id").as<int>();
        auto body = request.body();
        auto data = json::parse(body);

        // Create user with updated info
        User user{id, 
            data["name"].get<std::string>(),
            data["email"].get<std::string>(),
            data["bio"].get<std::string>(),
            data["title"].get<std::string>(),
            data["location"].get<std::string>(),
            data["skills"].get<std::string>(),
            data["hourlyRate"].get<std::string>(),
            data["availability"].get<std::string>(),
            data["experience"].get<std::string>(),
            data["education"].get<std::string>(),
            data["certifications"].get<std::string>(),
            data["githubUrl"].get<std::string>(),
            data["linkedinUrl"].get<std::string>(),
            data["portfolioUrl"].get<std::string>(),
            data["profilePicture"].get<std::string>()
        };

        // Add projects if present
        if (data.contains("projects") && data["projects"].is_array()) {
            for (const auto& projectData : data["projects"]) {
                Project project{
                    0,
                    projectData["name"].get<std::string>(),
                    projectData["description"].get<std::string>(),
                    projectData["techStack"].get<std::string>(),
                    projectData["role"].get<std::string>(),
                    projectData["startDate"].get<std::string>(),
                    projectData["endDate"].get<std::string>(),
                    projectData["projectUrl"].get<std::string>(),
                    projectData["isCurrent"].get<bool>()
                };
                user.projects.push_back(project);
            }
        }

        repository.updateUser(user);

        json responseJson = {
            {"message", "User updated successfully"},
            {"status", "success"}
        };
        response.send(Http::Code::Ok, responseJson.dump(), MIME(Application, Json));
    } catch (const json::exception& e) {
        json error = {
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
        response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to update user"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleSearchUsers(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto queryOpt = request.query().get("q");
        if (!queryOpt) {
            json error = {
                {"error", "Search query parameter 'q' is required"}
            };
            response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
            return;
        }

        std::string query = *queryOpt;
        if (query.empty()) {
            json error = {
                {"error", "Search query cannot be empty"}
            };
            response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
            return;
        }

        auto users = repository.searchUsers(query);
        json usersArray = json::array();

        for (const auto& user : users) {
            usersArray.push_back(user.to_json());
        }

        json responseJson;
        responseJson["system_message"] = "Search Results";
        responseJson["query"] = query;
        responseJson["count"] = users.size();
        responseJson["users"] = usersArray;

        response.send(Http::Code::Ok, responseJson.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to search users"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleAddProject(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto email = request.param(":email").as<std::string>();
        auto body = request.body();
        auto data = json::parse(body);

        Project project{
            0,
            data["name"].get<std::string>(),
            data["description"].get<std::string>(),
            data["techStack"].get<std::string>(),
            data["role"].get<std::string>(),
            data["startDate"].get<std::string>(),
            data["endDate"].get<std::string>(),
            data["projectUrl"].get<std::string>(),
            data["isCurrent"].get<bool>()
        };

        repository.addProjectToUser(email, project);

        json responseJson = {
            {"message", "Project added successfully"},
            {"status", "success"}
        };
        response.send(Http::Code::Created, responseJson.dump(), MIME(Application, Json));
    } catch (const json::exception& e) {
        json error = {
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
        response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to add project"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}

void UserController::handleUpdateProject(const Rest::Request& request, Http::ResponseWriter response) {
    try {
        auto email = request.param(":email").as<std::string>();
        auto projectId = request.param(":projectId").as<int>();
        auto body = request.body();
        auto data = json::parse(body);

        Project project{
            projectId,
            data["name"].get<std::string>(),
            data["description"].get<std::string>(),
            data["techStack"].get<std::string>(),
            data["role"].get<std::string>(),
            data["startDate"].get<std::string>(),
            data["endDate"].get<std::string>(),
            data["projectUrl"].get<std::string>(),
            data["isCurrent"].get<bool>()
        };

        repository.updateProject(email, project);

        json responseJson = {
            {"message", "Project updated successfully"},
            {"status", "success"}
        };
        response.send(Http::Code::Ok, responseJson.dump(), MIME(Application, Json));
    } catch (const json::exception& e) {
        json error = {
            {"error", "Invalid JSON format"},
            {"details", e.what()}
        };
        response.send(Http::Code::Bad_Request, error.dump(), MIME(Application, Json));
    } catch (const std::exception& e) {
        json error = {
            {"error", "Failed to update project"},
            {"details", e.what()}
        };
        response.send(Http::Code::Internal_Server_Error, error.dump(), MIME(Application, Json));
    }
}
