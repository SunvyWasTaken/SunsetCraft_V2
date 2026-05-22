cmake_minimum_required(VERSION 3.28)

project(SunsetCraftV2)

add_subdirectory(SunsetEngine)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC
        SunsetEngine
)
