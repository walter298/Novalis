cmake_minimum_required(VERSION 3.15)

project(novalis VERSION 0.1 LANGUAGES CXX)

# Require C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Gather source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS "src/*.h")

# Dependencies from vcpkg
find_package(Boost REQUIRED COMPONENTS type_erasure)
find_package(sdl3 CONFIG REQUIRED)

# Define library target
add_library(novalis STATIC ${SOURCES} ${HEADERS})
add_library(Novalis::novalis ALIAS novalis)

# Include path: allow users to do #include <Foo.h> or <detail/Bar.h>
target_include_directories(novalis
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)

# Link external libraries
target_link_libraries(novalis
    PRIVATE
        Boost::type_erasure
        SDL3::SDL3
)

# Install rules
install(TARGETS novalis
    EXPORT novalisTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY src/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT novalisTargets
    NAMESPACE Novalis::
    DESTINATION lib/cmake/novalis
)

# Optional: Add versioning config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/novalisConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/novalisConfigVersion.cmake"
    DESTINATION lib/cmake/novalis
)