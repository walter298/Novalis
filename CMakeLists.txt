cmake_minimum_required(VERSION 3.8)

project(Novalis)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PLF_INCLUDE "D:/Code_Libraries/plf-colony")

include_directories(
    include
    ${PLF_INCLUDE}
)

file(GLOB SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} STATIC ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY lib)