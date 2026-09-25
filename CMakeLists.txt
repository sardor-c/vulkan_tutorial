cmake_minimum_required(VERSION 3.20)

project(vulkan_tutorial CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(vulkan_tutorial src/main.cpp)

target_link_libraries(vulkan_tutorial PRIVATE Vulkan::Vulkan glfw glm::glm)