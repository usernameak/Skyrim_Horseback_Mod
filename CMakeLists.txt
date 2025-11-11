cmake_minimum_required(VERSION 3.20)
project(Horseback VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

find_package(CommonLibSSE CONFIG REQUIRED)
find_package(cpptoml CONFIG REQUIRED)

add_library(horseback SHARED
        Horseback.cpp
        PO3Helpers.hpp
        Config.hpp
        Config.cpp
        Module.hpp
        Module.cpp
        RoadStaminaModule.cpp
        RoadStaminaModule.hpp
        HorseInCellsModule.cpp
        HorseInCellsModule.hpp
)
target_link_libraries(horseback PRIVATE CommonLibSSE::CommonLibSSE cpptoml)
set_target_properties(horseback PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "D:/games/GOG/Skyrim Anniversary Edition/Data/SKSE/Plugins"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
