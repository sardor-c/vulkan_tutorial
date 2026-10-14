#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>


namespace utils {

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    );

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    );

    void printDeviceQueueFamilies(VkPhysicalDevice device);
    void printDeviceExtensions(const std::vector<VkExtensionProperties>& extensions);

    void printPhysicalDeviceVRAM(VkPhysicalDevice device);

    std::string createInfoLog(const std::string& matn);
    std::string createErrorLog(const std::string& matn);
    std::string CreateWarningLog(const std::string& matn);
}