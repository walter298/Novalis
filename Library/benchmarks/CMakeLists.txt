set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE BENCHMARK_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE BENCHMARK_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

message(STATUS "BENCHMARK_SOURCES = ${BENCHMARK_SOURCES}")

add_executable(novalis_benchmarks ${BENCHMARK_SOURCES} ${BENCHMARK_HEADERS})

target_link_libraries(novalis_benchmarks PRIVATE novalis)

target_include_directories(novalis_benchmarks
    PRIVATE ${CMAKE_SOURCE_DIR}/src
)