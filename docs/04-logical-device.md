# 4-dars: Logical device va graphics queue

**Natija:** dastur GPU bilan aloqa ochadi (logical device) va buyruqlar yuboriladigan navbatni (graphics queue) oladi. Konsolga `Logical device yaratildi, graphics queue olindi (oila 0)` chiqadi.

## Logical device nima?

3-darsda GPU'ni faqat **tanladik**, ya'ni physical device'dan ma'lumot so'radik. Endi u bilan **ishlash** uchun aloqa ochamiz:

| | Physical device | Logical device |
|---|---|---|
| Turi | `VkPhysicalDevice` | `VkDevice` |
| Nima | Haqiqiy GPU | Shu GPU bilan ochilgan "sessiya" |
| Qanday olinadi | `vkEnumerate...` orqali so'raladi | `vkCreateDevice` bilan yaratiladi |
| O'chiriladimi | Yo'q | **Ha**, `vkDestroyDevice` |
| Nimaga kerak | Tanlash uchun | Bundan keyingi **deyarli hamma narsa** shu orqali yaratiladi: buffer, tekstura, pipeline va h.k. |

Logical device yaratishda Vulkan'ga quyidagilarni aytamiz:
- **qaysi oiladan nechta navbat** kerakligi;
- GPU'ning **qaysi imkoniyatlari** (features) kerakligi;
- qaysi **device extension'lar** kerakligi.

Bitta physical device'dan bir nechta logical device yaratish mumkin, lekin odatda bittasi yetarli.

---

## 1-qadam: header

`HelloTriangleApplication.h` dagi class a'zolariga:

```cpp
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;          // YANGI
    VkQueue graphicsQueue = VK_NULL_HANDLE;    // YANGI
```

va funksiyalarga:

```cpp
    void createLogicalDevice();
    std::vector<const char*> getRequiredDeviceExtensions(VkPhysicalDevice device);
```

---

## 2-qadam: navbatlarni so'rash

`HelloTriangleApplication.cpp`:

```cpp
void HelloTriangleApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();
    createLogicalDevice();      // YANGI
}
```

```cpp
void HelloTriangleApplication::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
```

- **`queueFamilyIndex`**: 3-darsda topgan oila raqamimiz. `.value()` `std::optional` ichidagi qiymatni oladi.
- **`queueCount = 1`**: bu oiladan bitta navbat so'raymiz. Sizning GPU'da 0-oilada 16 ta navbat bor, lekin bizga bittasi yetarli. Bir nechta CPU oqimi (thread) bir vaqtda buyruq yubormoqchi bo'lsa, bir nechta navbat foydali bo'ladi.
- **`pQueuePriorities`**: har bir navbatning **ustuvorligi**, `0.0` dan `1.0` gacha. GPU yuqori ustuvorlikdagi navbatga ko'proq vaqt ajratishi mumkin. Navbat bitta bo'lsa ham bu maydon **shart**, shuning uchun bitta `float` ning manzilini beramiz. `queueCount = 2` bo'lsa, bu yerga 2 ta `float` dan iborat massiv berilardi.

---

## 3-qadam: imkoniyatlar (features)

```cpp
    VkPhysicalDeviceFeatures deviceFeatures{};
```

GPU'ning qo'shimcha imkoniyatlari. Masalan, `geometryShader`, `samplerAnisotropy` (teksturalarni sifatliroq ko'rsatish) yoki `wideLines` (qalin chiziqlar). Hozircha bizga hech biri kerak emas, shuning uchun `{}` bilan hammasini `VK_FALSE` qilamiz. Teksturalar darsida bu yerga qaytamiz.

Faqat **yoqilgan** imkoniyatdan foydalanish mumkin. GPU qo'llab-quvvatlasa ham, yoqilmagan imkoniyatni ishlatsangiz, validation layer xato beradi.

---

## 4-qadam: device extension'lar va macOS

Instance extension'lari (2-dars) **butun Vulkan** uchun edi. Device extension'lari esa **aynan shu GPU** uchun yoqiladi. Masalan, keyingi darslarda oynaga rasm chiqarish uchun `VK_KHR_swapchain` kerak bo'ladi.

Hozircha bitta holat bor: **macOS**. [macos.md](macos.md) da aytilganidek, MoltenVK GPU'si `VK_KHR_portability_subset` extension'ini e'lon qiladi. Bu extension "men Vulkan'ning ba'zi qismlarini qo'llab-quvvatlamayman" degani. Qoida shunday: **GPU bu extension'ni e'lon qilsa, uni yoqish majburiy.** Bu bilan dastur "men buni bilaman" deb tasdiqlaydi.

Avval `namespace { ... }` ga konstanta qo'shing:

```cpp
    constexpr const char* PORTABILITY_SUBSET_EXTENSION = "VK_KHR_portability_subset";
```

```cpp
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

    return extensions;
}
```

- **`vkEnumerateDeviceExtensionProperties`** instance'dagi `vkEnumerateInstanceExtensionProperties` ning device uchun o'xshashi. Unda ham "ikki marta chaqirish" uslubi ishlatiladi. Ikkinchi argument (`nullptr`) layer nomi; hamma extension'lar kerak bo'lgani uchun uni bo'sh qoldiramiz.
- **Nima uchun `#ifdef __APPLE__` emas?** Bu yerda `#ifdef` kerak emas. Biz GPU'ning o'zidan so'raymiz: extension bor bo'lsa, yoqamiz. Windows'dagi NVIDIA'da u yo'q, shuning uchun ro'yxat bo'sh qoladi. macOS'da u bor va yoqiladi. Bu usul har qanday platformada to'g'ri ishlaydi.
- **Nima uchun makros emas, matn?** `VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME` makrosi `vulkan_beta.h` faylida joylashgan. U faqat `VK_ENABLE_BETA_EXTENSIONS` e'lon qilinganda ulanadi. Oddiy matn bilan bu muammo bo'lmaydi.

---

## 5-qadam: logical device'ni yaratish

`createLogicalDevice()` ning davomi:

```cpp
    auto deviceExtensions = getRequiredDeviceExtensions(physicalDevice);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("Logical device yaratib bo'lmadi!");
    }
```

- Bu `VkInstanceCreateInfo` ga juda o'xshaydi: yana `sType`, yana extension'lar ro'yxati. **Vulkan'ning bir xil uslubi** shu.
- `pQueueCreateInfos` **massiv** kutadi. Bizda bitta element bor, shuning uchun bitta struct'ning manzili massiv vazifasini bajaradi. 5-darsda bu yerda ikkita element bo'ladi.
- **Layer'lar haqida.** Eski darsliklarda bu yerda `enabledLayerCount` va `ppEnabledLayerNames` ham to'ldiriladi. Bu maydonlar **eskirgan (deprecated)**: instance'da yoqilgan validation layer device chaqiruvlarini ham avtomatik tekshiradi. Shuning uchun ularni `{}` qoldirgan nol qiymatda qoldiramiz.

---

## 6-qadam: navbatni olish

`createLogicalDevice()` oxiriga:

```cpp
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
}
```

- Navbatlar `vkCreateDevice` paytida **avtomatik yaratiladi**. Biz faqat ulardan biriga "dastak" (handle) olamiz.
- Argumentlar: device, oila raqami, **navbat raqami** (shu oila ichida; bizda bittasi bor, shuning uchun `0`) va natija yoziladigan joy.
- Bu `vkGet...` funksiyasi, demak navbatni **o'chirish kerak emas**: u device bilan birga yo'qoladi (3-darsdagi qoida).

---

## 7-qadam: tozalash

Device instance'dan **keyin** yaratilgan, shuning uchun undan **oldin** o'chiriladi. `cleanup()` boshiga:

```cpp
void HelloTriangleApplication::cleanup() {
    vkDestroyDevice(device, nullptr);       // YANGI

    if (enableValidationLayers) {
        utils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
```

Yaratilish va o'chirilish tartibi:

```
yaratish:   oyna → instance → messenger → (physical device tanlash) → device
o'chirish:  device → messenger → instance → oyna
```

---

## Kutilgan natija

```
----[DEBUG]---- Validation layer yoqilgan!
----[DEBUG]---- Barcha kerakli extensionlar topildi!
----[DEBUG]---- VkInstance muvaffaqiyatli yaratildi!
----[DEBUG]---- Tanlangan GPU: NVIDIA GeForce RTX 3060 (rate: 132768 )
...
----[DEBUG]---- Logical device yaratildi, graphics queue olindi (oila 0)
```

Oxirgi qatorni o'zingiz `std::cout` bilan chiqaring.

---

## ✅ Sinov: `vkDestroyDevice` ni unutib ko'ring

`cleanup()` dagi `vkDestroyDevice` qatorini izohga oling, ishga tushiring va oynani X tugmasi bilan yoping:

```
----[ERROR]---- vkDestroyInstance(): VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT object VkDevice 0x... has not been destroyed.
The Vulkan spec states: All child objects that were created with instance or with a VkPhysicalDevice retrieved from it,
and that can be destroyed or freed, must have been destroyed or freed prior to destroying instance
(...#VUID-vkDestroyInstance-instance-00629)
```

Bu 2-darsdagi xatoning o'zi, faqat bu safar o'chirilmay qolgan obyekt `VkDevice`. Sinovdan keyin izohni olib tashlang.

---

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| Dastur `bad optional access` xabari bilan to'xtaydi | `graphicsFamily` bo'sh holda `.value()` chaqirilgan, `.value()` bunday holatda istisno (exception) otadi. `pickPhysicalDevice()` `createLogicalDevice()` dan oldin chaqirilganini tekshiring |
| `Logical device yaratib bo'lmadi!` | `sType` xato yozilgan yoki extension nomi noto'g'ri. Validation xabarini o'qing |
| `vkDestroyInstance(): ... VkDevice ... has not been destroyed` | `vkDestroyDevice` yo'q yoki `vkDestroyInstance` dan keyin chaqirilgan |
| `pQueuePriorities` haqida validation xatosi | `pQueuePriorities` berilmagan (`nullptr`) |

---

## 🧩 Mashqlar

### 1. Device extension'larni sanang

`getRequiredDeviceExtensions` da GPU nechta device extension qo'llab-quvvatlashini chiqaring. Ular orasida `VK_KHR_swapchain` (`VK_KHR_SWAPCHAIN_EXTENSION_NAME`) bor-yo'qligini tekshiring.

Sizning RTX 3060 da **276** ta extension bor va `VK_KHR_swapchain` ular orasida. Keyingi darslarda shu extension orqali oynaga rasm chiqaramiz.

### 2. Alohida compute navbati *(qiyinroq)*

Sizning GPU'da **2-oila** faqat `COMPUTE TRANSFER` qila oladi, ya'ni u GRAPHICS'siz, "toza" hisob-kitob oilasi. Bunday navbat chizish bilan parallel ravishda hisob-kitob qilish uchun foydali.

1. `QueueFamilyIndices` ga `std::optional<uint32_t> computeFamily;` qo'shing.
2. `findQueueFamilies` da **COMPUTE bor, lekin GRAPHICS yo'q** oilani toping:
   ```cpp
   (flags & VK_QUEUE_COMPUTE_BIT) && !(flags & VK_QUEUE_GRAPHICS_BIT)
   ```
   Agar bunday oila topilmasa, `computeFamily = graphicsFamily` qilib qo'ying, chunki GRAPHICS oilasi doim COMPUTE ham qila oladi.
3. `createLogicalDevice` da ikkita `VkDeviceQueueCreateInfo` ni `std::vector` ga yig'ing va `pQueueCreateInfos = vector.data()` qiling.
4. Class'ga `VkQueue computeQueue` qo'shib, uni `vkGetDeviceQueue` bilan oling.

**Tuzoq:** agar `computeFamily == graphicsFamily` bo'lsa, ikkita `VkDeviceQueueCreateInfo` bir xil oilani so'raydi va validation layer xato beradi. Bu sinab ko'rilgan:

```
vkCreateDevice(): pCreateInfo->pQueueCreateInfos[1] uses the same queueFamilyIndex (0) and flags ... as pQueueCreateInfos[0].
The Vulkan spec states: The combination of the values in the queueFamilyIndex and flags members of each element
of pQueueCreateInfos must be unique ... (VUID-VkDeviceCreateInfo-queueFamilyIndex-02802)
```

Shuning uchun oilalar **har xil** bo'lgandagina ikkinchi struct'ni qo'shing. Xuddi shu muammo 5-darsda ham uchraydi, u yerda uni chiroyliroq usul bilan hal qilamiz.
