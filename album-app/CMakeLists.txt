cmake_minimum_required(VERSION 3.16)
project(AlbumSharingService)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(PISTACHE REQUIRED libpistache)

find_path(JSONCPP_INCLUDE_DIR json/json.h PATHS /usr/include/jsoncpp /usr/local/include/jsoncpp)
find_library(JSONCPP_LIBRARY NAMES jsoncpp libjsoncpp)
if(NOT JSONCPP_INCLUDE_DIR OR NOT JSONCPP_LIBRARY)
    message(FATAL_ERROR "jsoncpp headers or library not found")
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PISTACHE_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIR}
)

set(SOURCES
    src/main.cpp
    src/models/Album.cpp
    src/models/Image.cpp
    src/services/AlbumService.cpp
    src/services/ImageService.cpp
    src/controllers/AlbumController.cpp
)

add_executable(album_sharing_service ${SOURCES})

target_link_libraries(album_sharing_service
    ${PISTACHE_LIBRARIES}
    ${JSONCPP_LIBRARY}
    ${OPENSSL_LIBRARIES}
    Threads::Threads
)

if(MSVC)
    target_compile_options(album_sharing_service PRIVATE /W4)
else()
    target_compile_options(album_sharing_service PRIVATE -Wall -Wextra -Wpedantic)
endif()
