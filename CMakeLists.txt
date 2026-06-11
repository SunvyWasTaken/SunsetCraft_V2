cmake_minimum_required(VERSION 3.28)

#! ! ! ! ! ! !
#set this to ON instead of OFF to ship the game!
#basically this will change the RESOURCES_PATH to be the local path
#! ! ! ! ! ! !
#delete the out folder after changing if visual studio doesn't recognize the change
set(PRODUCTION_BUILD ON CACHE BOOL "Make this a production build" FORCE)

project(SunsetCraftV2)

add_subdirectory(SunsetEngine)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Sources/*.cpp")

add_executable(${PROJECT_NAME} ${MY_SOURCES})

if(PRODUCTION_BUILD)
    target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_PATH=\"./Sources/Shaders/\")
else()
    target_compile_definitions(${PROJECT_NAME} PUBLIC SHADERS_PATH=\"${CMAKE_SOURCE_DIR}/Sources/Shaders/\")
endif()

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)
endif()
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 winmm)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC
        SunsetEngine
)
