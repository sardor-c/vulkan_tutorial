#include "HelloTriangleApplication.h"

#include <iostream>

int main() {
    HelloTriangleApplication application;

    try {
        application.run();
    }
    catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}