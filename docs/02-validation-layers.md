# 2-dars: Validation layer'lar

**Natija:** Vulkan'ni noto'g'ri ishlatsangiz, konsolga `[validation] ...` bilan boshlanadigan aniq xato xabari chiqadi.

## Nima uchun kerak?

Vulkan tez ishlashi uchun **deyarli hech narsani tekshirmaydi**. Noto'g'ri parametr bersangiz, xato chiqmasligi mumkin: dastur jimgina qulaydi, qora ekran ko'rsatadi yoki boshqa kompyuterda boshqacha ishlaydi.

**Validation layer** dasturingiz bilan drayver o'rtasiga qo'yiladigan "nazoratchi". U har bir `vk...` chaqiruvni Vulkan spetsifikatsiyasi bo'yicha tekshiradi va xato topsa, sizga xabar beradi.

```
Sizning kodingiz  →  [VK_LAYER_KHRONOS_validation]  →  NVIDIA drayveri
                              ↓
                     "Xato: siz X ni o'chirmadingiz"
```

- **Debug** rejimida layer'ni yoqamiz, chunki xatolarni topishimiz kerak.
- **Release** rejimida o'chiramiz, chunki tekshiruvlar dasturni sekinlashtiradi.

Bu darsda ikki ish qilamiz:
1. **Layer'ni yoqamiz.** Shunda tekshiruv ishlaydi.
2. **Debug messenger yaratamiz.** U layer'ning xabarlarini **o'zimizning funksiyamizga** yo'naltiradi, shuning uchun ularni qanday chiqarishni o'zimiz hal qilamiz.

---

## 1-qadam: header'ga yangi a'zolar

`HelloTriangleApplication.h`:

```cpp
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

class HelloTriangleApplication {
public:
    void run();

private:
    GLFWwindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;   // YANGI

    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    void createInstance();

    // YANGI
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
};
```

- `<vector>` header'da qoladi, chunki `getRequiredExtensions()` `std::vector` qaytaradi.
- `<iostream>` va `<stdexcept>` faqat `.cpp` da kerak, ularni o'sha yerga ko'chiring. `main.cpp` `std::cerr` ishlatadi, shuning uchun unga `#include <iostream>` qo'shing.
- `printExtensions()` endi kerak emas, uni o'chirsangiz ham bo'ladi.

---

## 2-qadam: sozlamalar

`HelloTriangleApplication.cpp` boshidagi `namespace { ... }` ga qo'shing:

```cpp
#include "HelloTriangleApplication.h"

#include <cstring>     // std::strcmp
#include <iostream>
#include <stdexcept>

namespace {
    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

#ifdef NDEBUG
    constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif
}
```

- **`VK_LAYER_KHRONOS_validation`**: Khronos'ning rasmiy validation layer'i. Ichida barcha tekshiruvlar bor.
- **`NDEBUG`**: CMake buni **Release** rejimida avtomatik e'lon qiladi, Debug'da esa e'lon qilmaydi. `#ifdef` kompilyatsiyadan oldin ishlaydi, shuning uchun bitta kod ikki rejimda ikki xil ishlaydi.

---

## 3-qadam: layer mavjudligini tekshirish

Layer o'rnatilmagan bo'lsa, `vkCreateInstance` tushunarsiz xato beradi. Shuning uchun avval o'zimiz tekshiramiz:

```cpp
bool HelloTriangleApplication::checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (std::strcmp(layerName, layerProperties.layerName) == 0) {
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
```

- Yana **"ikki marta chaqirish"** uslubi, bu safar layer'lar uchun.
- **`std::strcmp`**: `const char*` matnlarni `==` bilan solishtirib bo'lmaydi, chunki `==` matnni emas, **manzillarni** solishtiradi. `strcmp` matnlar bir xil bo'lsa `0` qaytaradi.
- Tashqi sikl bizga kerak bo'lgan har bir layer bo'yicha aylanadi, ichki sikl esa uni mavjud layer'lar orasidan qidiradi.

---

## 4-qadam: kerakli extension'lar ro'yxati

Debug messenger o'zi ham extension: `VK_EXT_debug_utils`. GLFW extension'lariga uni ham qo'shamiz:

```cpp
std::vector<const char*> HelloTriangleApplication::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return extensions;
}
```

- `vector(boshi, oxiri)`: oddiy massivdan vector yasaydi. `glfwExtensions + glfwExtensionCount` massiv oxiridan keyingi joyni ko'rsatadi.
- `VK_EXT_DEBUG_UTILS_EXTENSION_NAME` bu `"VK_EXT_debug_utils"` matni. Makrosdan foydalansangiz, nomni xato yozib qo'yish xavfi bo'lmaydi.

---

## 5-qadam: callback, ya'ni xabarlarni qabul qiluvchi funksiya

Layer xato topsa, **shu funksiyani chaqiradi**:

```cpp
VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "[validation] " << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}
```

- **`static`** so'zi **faqat header'da** yoziladi, `.cpp` da yozilmaydi. Aks holda kompilyator xato beradi.
- **Nima uchun `static`?** Vulkan C kutubxonasi, u oddiy funksiya pointer'ini kutadi. Oddiy a'zo funksiya esa yashirin `this` parametrini oladi, `static` funksiya olmaydi.
- **`VKAPI_ATTR` / `VKAPI_CALL`**: funksiya Vulkan kutgan "chaqirish konvensiyasi" bilan kompilyatsiya qilinishi uchun kerak. Header'da ham, `.cpp` da ham yoziladi.
- **`pCallbackData->pMessage`**: xabar matni. `->` pointer orqali struct maydoniga murojaat qiladi.
- **`return VK_FALSE`**: chaqiruvni to'xtatmasdan davom ettiradi. Har doim `VK_FALSE` qaytaring.
- **Parametrlar:**
  - `messageSeverity`: xabar qanchalik jiddiy (verbose, info, warning yoki error).
  - `messageType`: xabar turi (general, validation yoki performance).
  - `pUserData`: o'zingiz uzatadigan ixtiyoriy ma'lumot. Hozir kerak emas.

---

## 6-qadam: messenger sozlamalari

Bu struct ikki joyda kerak bo'ladi (7- va 8-qadamlar), shuning uchun uni alohida funksiyada to'ldiramiz:

```cpp
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
```

- **`&` (reference)**: funksiya chaqiruvchining struct'ini **to'g'ridan-to'g'ri** o'zgartiradi, nusxasini emas.
- **`|` (bitli YOKI)**: bir nechta "bayroq"ni (flag) birlashtiradi. Bu yerda Vulkan'ga "faqat warning va error xabarlarini yubor" deyapmiz.
- **`pfnUserCallback`**: `pfn` "pointer to function" degani. Bu maydonga 5-qadamdagi funksiyamizni beramiz.

---

## 7-qadam: `createInstance()` ni yangilash

```cpp
void HelloTriangleApplication::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layer so'raldi, lekin topilmadi!");
    }

    VkApplicationInfo appInfo{};
    // ... appInfo avvalgidek ...

    auto extensions = getRequiredExtensions();

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("VkInstance yaratib bo'lmadi!");
    }
    std::cout << "VkInstance muvaffaqiyatli yaratildi!" << std::endl;
}
```

- **`static_cast<uint32_t>(...)`**: `size()` `size_t` turini qaytaradi, Vulkan esa `uint32_t` kutadi. `static_cast` turni ochiq-oydin o'zgartiradi.
- **`pNext`**: Vulkan struct'larini "zanjir" qilib ulash usuli. Bu yerda u alohida vazifani bajaradi. Odatdagi messenger (8-qadam) instance **yaratilgandan keyin** paydo bo'ladi, shuning uchun u `vkCreateInstance` va `vkDestroyInstance` ichidagi xatolarni ko'ra olmaydi. `pNext` orqali ulangan vaqtinchalik messenger aynan shu ikki chaqiruvni kuzatadi.
- **`debugCreateInfo` nima uchun `if` dan tashqarida e'lon qilingan?** U `if` ichida e'lon qilinsa, `}` da yo'q bo'lib ketadi va `pNext` yo'q obyektga ishora qilib qoladi. `vkCreateInstance` chaqirilganda bu struct hali "tirik" bo'lishi kerak.

---

## 8-qadam: doimiy messenger yaratish

`vkCreateDebugUtilsMessengerEXT` **extension funksiyasi**, shuning uchun u Vulkan kutubxonasida avtomatik ravishda mavjud emas. Uning manzilini `vkGetInstanceProcAddr` orqali qo'lda so'raymiz. Ikkita yordamchi funksiyani `namespace { ... }` ichiga qo'shing:

```cpp
    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
            vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }
```

- `vkGetInstanceProcAddr` funksiyaning **nomini** olib, uning **manzilini** qaytaradi. Funksiya topilmasa, `nullptr` qaytaradi.
- `PFN_vk...` turi funksiya pointer'ining turi. `(TUR)` qavslari natijani shu turga o'giradi.

Endi messenger'ni yaratadigan funksiya:

```cpp
void HelloTriangleApplication::setupDebugMessenger() {
    if (!enableValidationLayers) {
        return;
    }

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Debug messenger yaratib bo'lmadi!");
    }
}
```

Uni instance'dan keyin chaqiring:

```cpp
void HelloTriangleApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
}
```

---

## 9-qadam: tozalash

Messenger instance'dan keyin yaratilgan, demak instance'dan **oldin** o'chiriladi:

```cpp
void HelloTriangleApplication::cleanup() {
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
```

---

## Kutilgan natija

```
VkInstance muvaffaqiyatli yaratildi!
[validation] windows_read_data_files_in_registry: Registry lookup failed to get layer manifest files.
```

Ikkinchi qator **zararsiz**. Vulkan loader layer'larni avval Windows registry'dan qidiradi va u yerda topolmaydi. Bizning layer esa `VK_ADD_LAYER_PATH` orqali topiladi. Bu xabarning chiqishi messenger ishlayotganini ham ko'rsatadi.

---

## ✅ Sinov: layer haqiqatan ishlayaptimi?

Ataylab xato qiling. `cleanup()` da messenger'ni o'chiradigan qatorni izohga oling:

```cpp
        // DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
```

Build qilib, ishga tushiring va oynani **X tugmasi bilan yoping**. Shunda `cleanup()` ishlaydi. Konsolga taxminan quyidagi xabar chiqishi kerak:

```
[validation] vkDestroyInstance(): VkInstance 0x... has 1 leaked objects that have not been destroyed.
VkDebugUtilsMessengerEXT 0x...
The Vulkan spec states: All child objects that were created with instance ... must have been
destroyed or freed prior to destroying instance (https://docs.vulkan.org/spec/latest/...#VUID-vkDestroyInstance-instance-00629)
```

Xabar ikki marta chiqadi: bir marta `pNext` messenger'idan, bir marta o'chirilmay qolgan doimiy messenger'dan.

**Validation xabarini o'qish:**
- `vkDestroyInstance()`: xato qaysi funksiyada topilgani.
- `has 1 leaked objects`: nima noto'g'ri.
- `VUID-...`: qoidaning raqami. Havolani ochsangiz, spetsifikatsiyadagi aniq qoidani ko'rasiz.

Sinovdan keyin izohni olib tashlashni unutmang!

---

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| `Validation layer so'raldi, lekin topilmadi!` | `VK_ADD_LAYER_PATH` o'rnatilmagan yoki terminal eski. VS Code'ni qayta oching |
| `cannot declare member function '...' to have static linkage` | `.cpp` dagi `debugCallback` oldida `static` yozilgan. Uni olib tashlang |
| `undefined reference to vkCreateDebugUtilsMessengerEXT` | Funksiya to'g'ridan-to'g'ri chaqirilgan. Katta harf bilan boshlanuvchi yordamchi `CreateDebugUtilsMessengerEXT` dan foydalaning |
| `strcmp was not declared` | `#include <cstring>` yo'q |
| Hech qanday `[validation]` xabari chiqmaydi | Release rejimida build qilingan (`NDEBUG` e'lon qilingan) yoki `pfnUserCallback` berilmagan |

---

## 🧩 Mashq

1. `populateDebugMessengerCreateInfo` da `messageSeverity` ga `VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT` ni qo'shing. Nechta xabar chiqadi? Ular nima haqida? Keyin uni yana olib tashlang, chunki juda ko'p xabar chiqadi.
2. `debugCallback` da faqat jiddiy xabarlarni chiqaring:
   ```cpp
   if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) { ... }
   ```
   Severity qiymatlari o'sib boradi: verbose < info < warning < error. Shuning uchun `>=` bilan solishtirish mumkin.
3. *(Qiyinroq)* Xabar oldiga darajasini yozing: `[ERROR]` yoki `[WARNING]`.
