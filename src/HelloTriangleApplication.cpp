#include "HelloTriangleApplication.h"

#include <iostream>
#include <stdexcept>
#include <cstring>
#include "color.h"
#include "utils.h"


namespace {
    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    constexpr const char* debugHeader = "----[ DEBUG ]----";

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    constexpr const char* PORTABILITY_SUBSET_EXTENSION = "VK_KHR_portability_subset";

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif

}


void HelloTriangleApplication::run() {
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void HelloTriangleApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();
}

void HelloTriangleApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

}

void HelloTriangleApplication::cleanup() {
    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        utils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void HelloTriangleApplication::createInstance() {

    if (enableValidationLayers) {
        std::cout << utils::createInfoLog("Validation layer yoqilgan!");
        if (!checkValidationLayerSupport()) {
            throw std::runtime_error("Validation layer yoqilgan, lekin topilmadi!");
        }
    }
    else {
        std::cout << utils::createInfoLog("Validation layer o'chirilgan!");
    }

    VkApplicationInfo appInfo{};

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 1);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    auto extensions = getRequiredExtensions();
    checkRequiredExtensions(extensions);

    VkInstanceCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("VkInstance yaratib bo'lmadi!");
    }
    std::cout << utils::createInfoLog("VkInstance muvaffaqiyatli yaratildi!");
}


bool HelloTriangleApplication::checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* required : validationLayers) {
        bool layerFound = false;
        for (const auto& available : availableLayers) {
            if (strcmp(required, available.layerName) == 0) {
                layerFound = true;
                break;
            }
        }
        if (!layerFound) {
            return false;
        }
    }

    return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

#ifdef __APPLE__
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif

    return extensions;
}

void HelloTriangleApplication::checkRequiredExtensions(const std::vector<const char*>& requiredExtensions) {
    uint32_t availableCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(availableCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availableCount, availableExtensions.data());
    bool hasError = false;
    for (const char* required : requiredExtensions) {
        bool found = false;
        for (const auto& available : availableExtensions) {
            if (strcmp(required, available.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << utils::createErrorLog(std::string(required) + " extension talab qilindi, lekin topilmadi!");
            hasError = true;
        }
    }
    if (hasError) {
        throw std::runtime_error("Kerakli Extensionlar topilmadi!");
    }
    else {
        std::cout << utils::createInfoLog("Barcha kerakli extensionlar topildi!");
    }
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

}

void HelloTriangleApplication::setupDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateDebugMessengerCreateInfo(createInfo);

    if (utils::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Debug messenger yaratib bo'lmadi");
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    (void)messageType;
    (void)pUserData;

    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        std::cerr << utils::createErrorLog(pCallbackData->pMessage);
    }
    else {
        std::cerr << utils::CreateWarningLog(pCallbackData->pMessage);
    }
    return VK_FALSE;
}

void HelloTriangleApplication::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Vulkan'ni qo'llab-quvvatlovchi GPU topilmadi!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    int highScore = 0;
    for (const auto& device : devices) {
        int rate = rateGpu(device);
        if (rate > highScore) {
            highScore = rate;
            physicalDevice = device;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Mos GPU topilmadi!");
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << color::YASHIL << debugHeader << "Tanlangan GPU: " << properties.deviceName << " (rate: " << highScore << " )" << color::RESET << std::endl;
    utils::printDeviceQueueFamilies(physicalDevice);
    utils::printPhysicalDeviceVRAM(physicalDevice);
}


int HelloTriangleApplication::rateGpu(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.hasGraphicsFamily()) {
        return 0;
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);

    int score = 0;
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 100000;
    }
    score += properties.limits.maxImageDimension2D;
    return score;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !indices.graphicsFamily.has_value()) {
            indices.graphicsFamily = i;
        }

        if ((queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !indices.computeFamily.has_value()) {
            indices.computeFamily = i;
        }


        if (indices.hasGraphicsFamily() && indices.hasComputeFamily()) {
            break;
        }
    }

    if (!indices.hasComputeFamily()) {
        indices.computeFamily = indices.graphicsFamily;
    }

    return indices;
}

void HelloTriangleApplication::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    float graphicQueuePriority = 1.0f;

    VkDeviceQueueCreateInfo graphicQueueCreateInfo{};
    graphicQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    graphicQueueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    graphicQueueCreateInfo.queueCount = 1;
    graphicQueueCreateInfo.pQueuePriorities = &graphicQueuePriority;


    float computeQueuePriority = 1.0f;

    VkDeviceQueueCreateInfo computeQueueCreateInfo{};
    computeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    computeQueueCreateInfo.queueFamilyIndex = indices.computeFamily.value();
    computeQueueCreateInfo.queueCount = 1;
    computeQueueCreateInfo.pQueuePriorities = &computeQueuePriority;

    VkPhysicalDeviceFeatures deviceFeatures{};

    auto deviceExtensions = getRequiredDeviceExtensions(physicalDevice);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.push_back(graphicQueueCreateInfo);
    if (indices.graphicsFamily.value() != indices.computeFamily.value()) {
        queueCreateInfos.push_back(computeQueueCreateInfo);
    }
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Logical device yaratib bo'lmadi!");
    }
    else {
        std::cout << utils::createInfoLog("Logical device yaratildi!");
    }

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    if (indices.graphicsFamily.value() != indices.computeFamily.value()) {
        vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
    }


}

std::vector<const char*> HelloTriangleApplication::getRequiredDeviceExtensions(VkPhysicalDevice device) {
    std::vector<const char*> extensions;

    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

    for (const auto& ext : available) {
        if (strcmp(ext.extensionName, PORTABILITY_SUBSET_EXTENSION) == 0) {
            extensions.push_back(PORTABILITY_SUBSET_EXTENSION);
        }
    }

    //utils::printDeviceExtensions(available);

    return extensions;
}



