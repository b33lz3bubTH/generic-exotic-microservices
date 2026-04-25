cmake_minimum_required(VERSION 3.10)
project(photo_sharing_service)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Drogon REQUIRED)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# Source files
set(SOURCES
    src/main.cpp
    src/controllers/AlbumController.cpp
    src/services/AlbumService.cpp
    src/services/ImageService.cpp
    src/models/Album.cpp
    src/models/Image.cpp
    src/utils/SlugGenerator.cpp
)

# Create executable
add_executable(blogging_app ${SOURCES})

# Link libraries
target_link_libraries(blogging_app 
    Drogon::Drogon
)

# Set compiler flags for better warnings
if(MSVC)
    target_compile_options(blogging_app PRIVATE /W4)
else()
    target_compile_options(blogging_app PRIVATE -Wall -Wextra -Wpedantic)
endif()
