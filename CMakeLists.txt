cmake_minimum_required(VERSION 3.28)

project(SunsetCraftV2)

add_subdirectory(SunsetEngine)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp")

add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_PATH=\"${CMAKE_SOURCE_DIR}/Sources/Shaders/\")

target_link_libraries(${PROJECT_NAME} PUBLIC
        SunsetEngine
)
