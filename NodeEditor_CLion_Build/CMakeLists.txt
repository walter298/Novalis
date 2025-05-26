cmake_minimum_required(VERSION 3.31)
project(NodeEditor_CLion_Build)

set(CMAKE_CXX_STANDARD 26)

add_executable(NodeEditor_CLion_Build main.cpp
        src/NodeEditor.ixx
        src/dependency_module_wrappers/sdl.ixx)
