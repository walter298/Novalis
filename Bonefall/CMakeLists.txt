cmake_minimum_required(VERSION 3.15)

project(Bonefall VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(novalis STATIC IMPORTED)

set_target_properties(novalis PROPERTIES
    IMPORTED_CONFIGURATIONS "Debug;Release"
    IMPORTED_LOCATION_RELEASE "${CMAKE_SOURCE_DIR}/../Library/build/Release/novalis.lib"
    IMPORTED_LOCATION_DEBUG   "${CMAKE_SOURCE_DIR}/../Library/build/Debug/novalis.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/../Library/src"
)

find_package(Boost REQUIRED COMPONENTS optional process)
find_package(magic_enum CONFIG REQUIRED)
find_package(sdl3 CONFIG REQUIRED)
find_package(sdl3_image CONFIG REQUIRED)
find_package(nfd CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE 
    novalis 
    Boost::optional 
    Boost::process
    magic_enum::magic_enum 
    SDL3::SDL3 
    SDL3_image::SDL3_image
    nfd::nfd
)