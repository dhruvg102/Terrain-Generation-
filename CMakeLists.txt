cmake_minimum_required(VERSION 3.13)

project(
    OpenGLDemo
    DESCRIPTION "OpenGL demo project for learning."
    LANGUAGES C CXX
)

# Assimp Options
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
option(BUILD_SHARED_LIBS OFF)
add_subdirectory(lib/assimp)

add_subdirectory(lib/glfw)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /MTd")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

include_directories(
    "include/"
    include/
    src/Utility/
    lib/
    lib/assimp/include/
    lib/glad/include/
    lib/glfw/include/
    lib/glm/
    lib/stb/
    lib/imgui/
    lib/imgui/backends/
)

file(
    GLOB VENDORS_SOURCES 
    lib/glad/src/glad.c
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_opengl3.cpp
    lib/imgui/imgui*.cpp
)

file(
    GLOB PROJECT_HEADERS
    include/*.hpp
    include/*.h
    src/Utility/*.h
)

file(
    GLOB PROJECT_SOURCES
    src/*.cpp
    src/Utility/*.cpp
)

file(
    GLOB PROJECT_SHADERS
    assets/shader/*.frag
    assets/shader/*.vert
    assets/shader/*.geom
)

source_group("headers" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("sources" FILES ${PROJECT_SOURCES})
source_group("vendors" FILES ${VENDORS_SOURCES})

set(APP_NAME OpenGL_RENDER)
file(
    GLOB APP_SOURCES
    src/main.cpp
)

# 链接可执行文件
add_executable(${APP_NAME} ${APP_SOURCES} ${PROJECT_HEADERS}
                             ${PROJECT_SHADERS} ${VENDORS_SOURCES})

# 链接库
target_link_libraries(${APP_NAME} assimp glfw ${GLAD_LIBRARIES})

# 设置运行时项目输出目录位置
set_target_properties(${APP_NAME} PROPERTIES
                      RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${APP_NAME})