#include "utils.h"

#include <vector>
#include <iostream>
#include "color.h"

namespace utils {
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    ) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    void printDeviceQueueFamilies(VkPhysicalDevice device) {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());


        std::cout << color::OQ << color::QALIN << "----------------[ Queue Families] ------------------" << std::endl;
        for (uint32_t i = 0; i < queueFamilies.size(); i++) {
            const auto& f = queueFamilies[i];
            std::cout << color::RESET << color::OQ << "|   " << color::RESET << color::YASHIL;
            std::cout << "oila " << i << ": " << f.queueCount << " ta navbat |"
                << ((f.queueFlags & VK_QUEUE_GRAPHICS_BIT) ? " GRAPHICS" : "")
                << ((f.queueFlags & VK_QUEUE_COMPUTE_BIT) ? " COMPUTE" : "")
                << ((f.queueFlags & VK_QUEUE_TRANSFER_BIT) ? " TRANSFER" : "")
                << std::endl;
        }
        std::cout << color::OQ << color::QALIN << "----------------------------------------------------" << color::RESET << std::endl;
    }

    void printPhysicalDeviceVRAM(VkPhysicalDevice device) {
        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);

        for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                std::cout << color::OQ << color::QALIN << "----------------[ VRAM ] ------------------" << std::endl;
                std::cout << color::RESET << color::OQ << "|   " << color::RESET << color::YASHIL;
                std::cout << "          " << memProps.memoryHeaps[i].size / (1024.0 * 1024.0 * 1024.0) << " GB" << std::endl;
                std::cout << color::OQ << color::QALIN << "-------------------------------------------" << color::RESET << std::endl;
            }
        }
    }
}