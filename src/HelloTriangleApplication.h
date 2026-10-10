#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

class HelloTriangleApplication {

public:
    void run();

private:
    GLFWwindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();


    void createInstance();
    void printExtensions();

};