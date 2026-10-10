#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>


class HelloTriangleApplication {

public:
    void run();

private:
    GLFWwindow* window;
    VkInstance instance = VK_NULL_HANDLE;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

private:
    void createInstance();
    void printExtensions();

};