cmake_minimum_required(VERSION 3.10)
project(FreelanceService)

set(CMAKE_CXX_STANDARD 17)

find_package(SQLite3 REQUIRED)

include_directories(
    ${SQLite3_INCLUDE_DIRS}
    db
    models
    repositories
    controllers
    plugins
    plugs
)

add_executable(FreelanceService
    main.cpp
    db/Database.cpp
    db/DatabaseUtils.cpp
    models/LogEntry.cpp
    models/VaultEntry.cpp
    models/ContactUsEntry.cpp
    repositories/LogRepository.cpp
    repositories/VaultRepository.cpp
    repositories/ContactUsRepository.cpp
    controllers/LogController.cpp
    controllers/VaultController.cpp
    controllers/ContactUsController.cpp
    plugins/PluginManager.cpp
    plugs/RackSackPlugin.cpp
)

target_link_libraries(FreelanceService
    ${SQLite3_LIBRARIES}
    pistache
    ssl
    crypto
)