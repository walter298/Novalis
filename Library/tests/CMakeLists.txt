set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE TEST_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

message(STATUS "TEST_SOURCES = ${BENCHMARK_SOURCES}")

add_executable(novalis_tests ${TEST_SOURCES} ${TEST_HEADERS} "BufferedNodeTests.h" "BufferedNodeTests.cpp")

target_link_libraries(novalis_tests PRIVATE novalis)

target_include_directories(novalis_tests
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)