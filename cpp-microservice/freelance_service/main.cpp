#include <iostream>
#include <httplib.h>
#include "controllers/UserController.h"
#include "db/Database.h"
#include "db/DatabaseUtils.h"

int main() {
    try {
        Database::getInstance(DatabaseUtils::DB_NAME);
        httplib::Server svr;
        
        UserController userController(svr);
        userController.setupRoutes();
        
        std::cout << "Server starting on port 8080..." << std::endl;
        svr.listen("0.0.0.0", 8080);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
} 