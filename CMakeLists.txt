cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(image-binary-matrix CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(LIBS_PATH ${PROJECT_SOURCE_DIR}/libs)
set(SOURCE_PATH ${PROJECT_SOURCE_DIR}/src)

find_package(glew REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(
    ${LIBS_PATH}/glew
    ${LIBS_PATH}/glfw
    ${LIBS_PATH}/imgui
    ${LIBS_PATH}/opencv
)
add_executable(${PROJECT_NAME} ${SOURCE_PATH}/main.cpp)

target_compile_definitions(${PROJECT_NAME} PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
target_link_libraries(${PROJECT_NAME} GLEW::GLEW glfw imgui::imgui opencv::opencv)
