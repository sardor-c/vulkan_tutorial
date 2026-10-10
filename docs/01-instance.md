# 1-dars: VkInstance yaratish

**Natija:** konsolga `VkInstance muvaffaqiyatli yaratildi!` chiqadi va bo'sh oyna ochiladi.

## Instance nima?

**VkInstance** dasturingiz bilan Vulkan kutubxonasi o'rtasidagi birinchi aloqa. U orqali Vulkan'ga quyidagilarni aytasiz:

- dastur nomi va qaysi Vulkan versiyasidan foydalanishi;
- qaysi **extension**'lar kerakligi (masalan, Windows oynasiga chizish uchun);
- qaysi **layer**'lar yoqilishi (masalan, validation, 2-darsga qarang).

GPU tanlash, chizish va boshqa hamma narsa instance'dan keyin keladi.

## Vulkan'ning asosiy uslubi

Deyarli har bir Vulkan obyekti bir xil tartibda yaratiladi:

```
1. Struct to'ldirasiz        →  VkXxxCreateInfo info{};
2. Funksiyaga berasiz        →  vkCreateXxx(&info, nullptr, &obyekt);
3. Natijani tekshirasiz      →  VK_SUCCESS bo'ldimi?
4. Oxirida o'chirasiz        →  vkDestroyXxx(obyekt, nullptr);
```

---

## 1-qadam: kodni class'ga o'tkazish

Kod tez orada 1000 qatordan oshadi, shuning uchun uni class'ga o'tkazamiz va uchta faylga bo'lamiz:

```
src/
├── main.cpp                       ← faqat class'ni ishga tushiradi
├── HelloTriangleApplication.h     ← class e'loni (nimalar bor)
└── HelloTriangleApplication.cpp   ← funksiyalarning tanasi (qanday ishlaydi)
```

`CMakeLists.txt` da yangi `.cpp` faylni ham ko'rsating:

```cmake
add_executable(vulkan_tutorial src/main.cpp src/HelloTriangleApplication.cpp)
```

### HelloTriangleApplication.h

```cpp
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

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
};
```

### HelloTriangleApplication.cpp

```cpp
#include "HelloTriangleApplication.h"

#include <iostream>
#include <stdexcept>

namespace {
    constexpr uint32_t WIDTH = 800;
    constexpr uint32_t HEIGHT = 600;
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
}

void HelloTriangleApplication::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void HelloTriangleApplication::cleanup() {
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void HelloTriangleApplication::createInstance() {
    // 2-qadamda to'ldiramiz
}
```

### main.cpp

```cpp
#include "HelloTriangleApplication.h"

#include <cstdlib>
#include <iostream>

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

### C++ tomondan yangiliklar

| Narsa | Ma'nosi |
|---|---|
| `class` | Ma'lumotlar (`window`, `instance`) va funksiyalarni bitta joyga yig'adi. Class ichidagi har bir funksiya a'zolarni ko'radi |
| `public:` / `private:` | `run()` tashqaridan chaqiriladi, qolgani ichki ish |
| `#pragma once` | Header bitta faylga ikki marta ulanib qolmasligini ta'minlaydi |
| `Class::funksiya()` | `.cpp` faylda funksiyaning qaysi class'ga tegishli ekanini bildiradi |
| `namespace { ... }` | Ichidagilar faqat shu `.cpp` faylda ko'rinadi |
| `constexpr` | Kompilyatsiya vaqtida ma'lum bo'lgan o'zgarmas qiymat |
| `try` / `catch` | `throw` bilan "otilgan" xatoni ushlab, matnini chiqaradi |
| `VK_NULL_HANDLE` | Vulkan obyektining "bo'sh" qiymati, `nullptr` ga o'xshaydi |

**Qoida:** a'zo o'zgaruvchilarga doim boshlang'ich qiymat bering (`= nullptr`, `= VK_NULL_HANDLE`). Aks holda ularda tasodifiy "axlat" qiymat qoladi.

### Vulkan tomondan yangiliklar

- `GLFW_RESIZABLE, GLFW_FALSE`: hozircha oyna o'lchamini o'zgartirib bo'lmaydi. Buning uchun alohida kod kerak, uni keyinroq yozamiz.
- `run()` dasturning butun hayoti: **oyna → Vulkan → sikl → tozalash**.
- `cleanup()` da **teskari tartib**: instance oynadan keyin yaratilgan, shuning uchun oynadan oldin o'chiriladi.

---

## 2-qadam: `createInstance()`

```cpp
void HelloTriangleApplication::createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;
    createInfo.enabledLayerCount = 0;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("VkInstance yaratib bo'lmadi!");
    }

    std::cout << "VkInstance muvaffaqiyatli yaratildi!" << std::endl;
}
```

### Qatorma-qator

- **`{}`** struct'ning hamma maydonlarini nolga tenglaydi. **Hech qachon unutmang!**
- **`sType`**: deyarli har bir Vulkan struct'ida bor, unga struct'ning "kimligi" yoziladi. Nomi struct nomiga mos: `VkApplicationInfo` → `VK_STRUCTURE_TYPE_APPLICATION_INFO`.
- **`apiVersion`**: qaysi Vulkan versiyasidan foydalanamiz. RTX 3060 1.4 ni qo'llab-quvvatlaydi, darslar 1.3 ga asoslangan.
- **`glfwGetRequiredInstanceExtensions`**: Vulkan Windows oynalari haqida hech narsa bilmaydi. GLFW kerakli extension'larni (`VK_KHR_surface`, `VK_KHR_win32_surface`) aytib beradi.
- **`p` / `pp` prefikslari**: `p` pointer degani, `pp` esa pointer'ga pointer.
- **`pApplicationInfo = &appInfo`**: struct'lar bir-biriga manzil orqali ulanadi.
- **`vkCreateInstance(&createInfo, nullptr, &instance)`**: argumentlar sozlamalar, allocator (deyarli har doim `nullptr`) va natija yoziladigan joy. Funksiya `VkResult` qaytaradi, `VK_SUCCESS` dan boshqa qiymat xato degani.

---

## 🧩 Mashq: extension nomlarini chiqarish

"Ikki marta chaqirish" uslubi:

```cpp
#include <vector>

uint32_t count = 0;
vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);              // 1: soni

std::vector<VkExtensionProperties> extensions(count);                          // joy ajratish
vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data());    // 2: ma'lumot

for (const auto& ext : extensions) {
    std::cout << "\t" << ext.extensionName << std::endl;
}
```

`std::vector` o'lchami dastur ishlayotganda belgilanadigan massiv. `.data()` Vulkan kutadigan oddiy pointer'ni beradi.

Ro'yxatda `VK_KHR_surface` va `VK_KHR_win32_surface` borligini tekshiring. ✅
