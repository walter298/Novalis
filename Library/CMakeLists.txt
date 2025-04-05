CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

PROJECT(novalis)

SET(CMAKE_CXX_STANDARD 23)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

FILE(GLOB SOURCES "src/*.cpp")
FILE(GLOB HEADERS "src/*.h")

#FIND_PACKAGE(Boost REQUIRED COMPONENTS type_erasure geometry pfr unordered functional)
find_package(boost_type_erasure CONFIG REQUIRED)
find_package(boost_geometry CONFIG REQUIRED)
find_package(boost_pfr CONFIG REQUIRED)
find_package(boost_unordered CONFIG REQUIRED)
find_package(boost_functional CONFIG REQUIRED)
FIND_PACKAGE(sdl3 CONFIG REQUIRED)

ADD_LIBRARY(novalis STATIC ${SOURCES})

TARGET_INCLUDE_DIRECTORIES(novalis PUBLIC ${SOURCES})
target_link_libraries(
    novalis PRIVATE 
    boost_type_erasure
    boost_geometry
    boost_pfr
    boost_unordered
    boost_functional
    SDL3::SDL3
)