cmake_minimum_required(VERSION 3.10)
project(AlbumSharingService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(SQLite3 REQUIRED)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${SQLite3_INCLUDE_DIRS}
    include/db
    include/models
    include/repositories
    include/services
    include/controllers
    include/utils
)

# Source files
set(SOURCES
    src/main.cpp
    src/db/Database.cpp
    src/db/DatabaseUtils.cpp
    src/models/Album.cpp
    src/models/Image.cpp
    src/models/User.cpp
    src/models/Share.cpp
    src/repositories/AlbumRepository.cpp
    src/repositories/ImageRepository.cpp
    src/repositories/UserRepository.cpp
    src/repositories/ShareRepository.cpp
    src/services/AlbumService.cpp
    src/services/ImageService.cpp
    src/services/UserService.cpp
    src/services/ShareService.cpp
    src/controllers/AlbumController.cpp
    src/controllers/ImageController.cpp
    src/controllers/UserController.cpp
    src/controllers/ShareController.cpp
    src/utils/SlugGenerator.cpp
    src/utils/Utils.cpp
)

# Create executable
add_executable(album_sharing_service ${SOURCES})

# Link libraries
target_link_libraries(album_sharing_service
    ${SQLite3_LIBRARIES}
    pistache
    ${OPENSSL_LIBRARIES}
)

# Set compiler flags
if(MSVC)
    target_compile_options(album_sharing_service PRIVATE /W4)
else()
    target_compile_options(album_sharing_service PRIVATE -Wall -Wextra -Wpedantic)
endif()
