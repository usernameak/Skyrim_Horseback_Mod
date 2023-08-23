cmake_minimum_required(VERSION 3.20)
project(Horseback VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

find_package(CommonLibSSE CONFIG REQUIRED)

add_library(horseback SHARED Horseback.cpp)
target_link_libraries(horseback PRIVATE CommonLibSSE::CommonLibSSE)
set_target_properties(horseback PROPERTIES
        # RUNTIME_OUTPUT_DIRECTORY "D:/games/The Elder Scrolls - Skyrim/Data/SKSE/Plugins"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        )
