cmake_minimum_required(VERSION 3.15)

project(novalis VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

# find_package(Boost REQUIRED COMPONENTS type_erasure)
#find_package(sdl3 CONFIG REQUIRED)

add_library(novalis STATIC ${SOURCES} ${HEADERS})
add_library(Novalis::novalis ALIAS novalis)

target_include_directories(novalis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# target_link_libraries(novalis
#     PRIVATE
#         #Boost::type_erasure
#         SDL3::SDL3
# )

include(CMakePackageConfigHelpers)

install(TARGETS novalis
    EXPORT novalisTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY src/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT novalisTargets
    FILE novalisTargets.cmake
    NAMESPACE Novalis::
    DESTINATION share/novalis
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/novalisConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfig.cmake"
    INSTALL_DESTINATION share/novalis
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfigVersion.cmake"
    DESTINATION share/novalis
)