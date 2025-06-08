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
)

add_executable(FreelanceService
    main.cpp
    db/Database.cpp
    db/DatabaseUtils.cpp
    models/LogEntry.cpp
    models/ServiceLogger.h
    repositories/LogRepository.cpp
    controllers/LogController.cpp
)

target_link_libraries(FreelanceService
    ${SQLite3_LIBRARIES}
    pistache
)