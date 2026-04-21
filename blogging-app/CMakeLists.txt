cmake_minimum_required(VERSION 3.10)
project(blogging_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Drogon REQUIRED)
find_package(mongodb REQUIRED)
find_package(mongocxx REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/controllers/BlogController.cpp
    src/services/BlogService.cpp
    src/utils/SlugGenerator.cpp
)

# Create executable
add_executable(blogging_app ${SOURCES})

# Link libraries
target_link_libraries(blogging_app 
    Drogon::Drogon
    mongo::mongocxx_shared
    mongo::bsoncxx_shared
)

# Set compiler flags for better warnings
if(MSVC)
    target_compile_options(blogging_app PRIVATE /W4)
else()
    target_compile_options(blogging_app PRIVATE -Wall -Wextra -Wpedantic)
endif()
