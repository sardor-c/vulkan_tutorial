# 6-dars: Swapchain

**Natija:** ekranga chiqariladigan rasmlar zanjiri yaratiladi. Konsolga quyidagi qator chiqadi:

```
Swapchain yaratildi: 3 ta rasm, 800x600, VK_FORMAT_B8G8R8A8_SRGB, VK_PRESENT_MODE_MAILBOX_KHR
```

## Swapchain nima?

Agar GPU to'g'ridan-to'g'ri ekrandagi rasmga chizsa, monitor **yarim chizilgan** kadrni ko'rsatib qo'yishi mumkin. Shuning uchun bir nechta rasm ishlatiladi: GPU bittasiga chizadi, ekran esa boshqasini ko'rsatadi. Chizish tugagach, ular **almashtiriladi** (swap).

```
            ┌──────────┐   ┌──────────┐   ┌──────────┐
swapchain:  │ rasm 0   │   │ rasm 1   │   │ rasm 2   │
            │ ekranda  │   │ GPU      │   │ navbatda │
            │ ko'rinadi│   │ chizyapti│   │ kutyapti │
            └──────────┘   └──────────┘   └──────────┘
                 ▲               │
                 └── present ◄───┘   (chizish tugagach)
```

**Swapchain** (`VkSwapchainKHR`) shu rasmlar to'plami va ularni almashtirish qoidasi. OpenGL'da bu ish yashirin bajarilardi (`glfwSwapBuffers`). Vulkan'da esa uni o'zingiz yaratasiz va sozlaysiz.

Swapchain yaratishdan oldin uchta savolga javob berish kerak:

| Savol | Sozlama | Bizning tanlov |
|---|---|---|
| Piksel ranglari qanday saqlanadi? | **surface format** | `B8G8R8A8_SRGB` |
| Rasmlar ekranga qachon chiqadi? | **present mode** | `MAILBOX`, bo'lmasa `FIFO` |
| Rasm o'lchami qancha? | **extent** | oyna o'lchami (800×600) |

---

## 1-qadam: `VK_KHR_swapchain` extension'i

Swapchain Vulkan'ning yadrosida yo'q, u **device extension**. Har bir GPU uni qo'llab-quvvatlashi shart emas. Masalan, faqat hisob-kitob uchun mo'ljallangan serverdagi GPU'da u bo'lmaydi. 4-darsdagi mashqda sizning GPU'da u borligini ko'rgan edingiz.

`namespace { ... }` ga majburiy device extension'lar ro'yxatini qo'shing:

```cpp
    const std::vector<const char*> requiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
```

`getRequiredDeviceExtensions` endi bo'sh ro'yxatdan emas, shu ro'yxatdan boshlanadi:

```cpp
std::vector<const char*> HelloTriangleApplication::getRequiredDeviceExtensions(VkPhysicalDevice device) {
    std::vector<const char*> extensions = requiredDeviceExtensions;   // O'ZGARDI
    // ... portability_subset qismi avvalgidek ...
}
```

`= requiredDeviceExtensions` vector'ni **nusxalaydi**. Shuning uchun keyin unga `push_back` qilinsa, asl ro'yxat o'zgarmaydi.

GPU tanlashda extension borligini tekshiring. Header'ga `bool checkDeviceExtensionSupport(VkPhysicalDevice device);` qo'shing:

```cpp
bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);

    std::vector<VkExtensionProperties> available(count);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &count, available.data());

    for (const char* required : requiredDeviceExtensions) {
        bool found = false;
        for (const auto& ext : available) {
            if (strcmp(required, ext.extensionName) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
    }
    return true;
}
```

Bu 2-darsdagi `checkValidationLayerSupport` bilan bir xil tuzilishga ega, faqat bu safar device extension'lari tekshiriladi.

---

## 2-qadam: surface nimalarni qo'llab-quvvatlaydi?

Extension borligining o'zi yetarli emas. GPU **bizning oynamiz** uchun qanday rasmlar yarata olishini so'rash kerak. Header'ga, `QueueFamilyIndices` dan keyin:

```cpp
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};          // rasm soni va o'lchami chegaralari
    std::vector<VkSurfaceFormatKHR> formats;          // qo'llab-quvvatlanadigan formatlar
    std::vector<VkPresentModeKHR> presentModes;       // qo'llab-quvvatlanadigan present mode'lar
};
```

va class'ga `SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);` qo'shing:

```cpp
SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());

    return details;
}
```

- Uchala funksiya ham `device` va `surface` ni oladi, chunki javob **ikkalasiga** bog'liq.
- `resize(n)` mavjud vector'ning o'lchamini o'zgartiradi. Bu `std::vector<T> v(n)` bilan bir xil, faqat vector allaqachon e'lon qilingan.

`rateGpu` ga ikkita yangi shart qo'shing. Ikkalasi ham present tekshiruvidan keyin keladi:

```cpp
    if (!checkDeviceExtensionSupport(device)) {
        return 0;
    }

    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        return 0;
    }
```

**Tartib muhim:** `querySwapChainSupport` ni faqat extension borligi tekshirilgandan **keyin** chaqirish mumkin.

---

## 3-qadam: format tanlash

Sizning GPU 7 ta formatni taklif qiladi (sinovdan olingan):

```
VK_FORMAT_B8G8R8A8_UNORM            / SRGB_NONLINEAR
VK_FORMAT_B8G8R8A8_SRGB             / SRGB_NONLINEAR    ← tanlaymiz
VK_FORMAT_R8G8B8A8_UNORM            / SRGB_NONLINEAR
VK_FORMAT_R8G8B8A8_SRGB             / SRGB_NONLINEAR
VK_FORMAT_R16G16B16A16_SFLOAT       / EXTENDED_SRGB_LINEAR   (HDR)
VK_FORMAT_A2B10G10R10_UNORM_PACK32  / HDR10_ST2084           (HDR)
VK_FORMAT_A2B10G10R10_UNORM_PACK32  / SRGB_NONLINEAR
```

Nomini o'qish: `B8G8R8A8` ko'k, yashil, qizil va shaffoflik (alpha) kanallari degani, har biri 8 bitdan (jami 32 bit). `_SRGB` qismi esa ranglar **sRGB** egri chizig'i bo'yicha saqlanishini bildiradi.

### sRGB nima uchun?

Inson ko'zi qorong'i ranglardagi farqni yorug' ranglardagidan yaxshiroq sezadi. sRGB formati 8 bitning ko'proq qismini qorong'i ranglarga ajratadi. Natijada rasm tabiiyroq ko'rinadi, monitorlar ham aynan shu formatni kutadi. `_SRGB` formatda GPU bu aylantirishni **avtomatik** bajaradi.

```cpp
VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& format : availableFormats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }
    return availableFormats[0];
}
```

Kerakli format topilmasa, birinchisini olamiz. `availableFormats[0]` xavfsiz, chunki 2-qadamda ro'yxat bo'sh emasligini tekshirgan edik.

---

## 4-qadam: present mode tanlash

Present mode tayyor rasm ekranga **qachon** chiqishini belgilaydi. Monitor ekranni sekundiga 60, 144 va hokazo marta yangilaydi. Har bir yangilanish orasidagi lahza **vertical blank** (vblank) deyiladi.

| Mode | Qanday ishlaydi | Natija |
|---|---|---|
| `IMMEDIATE` | Rasm darhol chiqadi, vblank kutilmaydi | Eng tez, lekin ekranda "yirtilish" (tearing) bo'lishi mumkin |
| `FIFO` | Rasmlar navbatda turadi, har vblank'da bittadan chiqadi | Bu V-Sync. Yirtilish yo'q. **Har doim mavjud** |
| `FIFO_RELAXED` | `FIFO` ga o'xshaydi, lekin kechikkan rasm darhol chiqadi | Ba'zan yirtilish bo'ladi |
| `MAILBOX` | Navbat to'lsa, eski rasm yangisi bilan **almashtiriladi** | Yirtilish yo'q va kechikish kam. "Triple buffering" |

Sizning GPU 5 ta mode'ni qo'llab-quvvatlaydi: `FIFO`, `FIFO_RELAXED`, `MAILBOX`, `IMMEDIATE` va `FIFO_LATEST_READY`. Oxirgisi yangi extension bilan qo'shilgan mode.

```cpp
VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& mode : availablePresentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}
```

`FIFO` ni tekshirmasdan qaytaramiz, chunki Vulkan spetsifikatsiyasi uni **har doim** qo'llab-quvvatlanishini kafolatlaydi.

---

## 5-qadam: rasm o'lchami (extent)

```cpp
#include <algorithm>   // std::clamp
#include <limits>      // std::numeric_limits
```

```cpp
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };

    actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

    return actualExtent;
}
```

- **Odatiy holat:** oyna tizimi rasm o'lchamini o'zi aytadi (`currentExtent`). Sizda bu `800x600`.
- **Maxsus holat:** ba'zi tizimlar (masalan, Wayland) `currentExtent` ga eng katta `uint32_t` qiymatini yozadi. Bu "o'lchamni o'zing tanla" degani. Bunday holatda oynaning **piksellardagi** o'lchamini olamiz.
- **Nima uchun `glfwGetFramebufferSize`, `WIDTH`/`HEIGHT` emas?** Yuqori aniqlikdagi ekranlarda (Retina, Windows'da 150% masshtab) oyna o'lchami va piksellar soni farq qiladi. `800x600` oyna `1600x1200` pikselga ega bo'lishi mumkin. Vulkan esa piksellar bilan ishlaydi.
- **`std::clamp(x, min, max)`** `x` ni `[min, max]` oralig'iga "qisib" qo'yadi.

---

## 6-qadam: swapchain yaratish

Class a'zolariga:

```cpp
    VkSwapchainKHR swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D swapChainExtent{};
```

va `void createSwapChain();`. Uni `initVulkan` da `createLogicalDevice()` dan keyin chaqiring.

```cpp
void HelloTriangleApplication::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
```

### Nechta rasm?

Sizning GPU'da `minImageCount = 2` va `maxImageCount = 8`. Minimal miqdorni olsak, GPU ba'zan drayver rasmni bo'shatishini kutib qoladi. Shuning uchun **bitta ortiqcha** rasm so'raymiz: `2 + 1 = 3`. `maxImageCount = 0` "chegara yo'q" degani, shuning uchun `> 0` tekshiruvi kerak.

```cpp
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
```

- **`imageArrayLayers = 1`**: har bir rasm bitta qatlamdan iborat. Bu qiymat faqat stereo 3D (VR) uchun 2 bo'ladi.
- **`imageUsage`**: rasmdan nima uchun foydalanamiz. `COLOR_ATTACHMENT`: unga to'g'ridan-to'g'ri **chizamiz**.

### Navbatlar orasida bo'lishish

```cpp
    uint32_t familyIndices[] = {
        queueFamilyIndices.graphicsFamily.value(),
        queueFamilyIndices.presentFamily.value()
    };

    if (queueFamilyIndices.graphicsFamily.value() != queueFamilyIndices.presentFamily.value()) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = familyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }
```

Swapchain rasmlariga GRAPHICS oilasi **chizadi**, present oilasi esa ularni ekranga **chiqaradi**.
- **`EXCLUSIVE`**: rasm bir vaqtda bitta oilaga tegishli bo'ladi. Bu eng tez rejim. Sizda graphics va present bitta oilada (0), shuning uchun shu rejim tanlanadi.
- **`CONCURRENT`**: rasmni bir nechta oila erkin ishlatadi. Bu sekinroq, lekin oilalar har xil bo'lganda eng sodda yechim.

⚠️ **`familyIndices` `if` dan tashqarida e'lon qilingan.** Sababi yana o'sha: `pQueueFamilyIndices` `vkCreateSwapchainKHR` chaqirilgunga qadar mavjud bo'lgan massivga ishora qilishi kerak. Bu 2-, 4- va 5-darslardagi qoida.

```cpp
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Swapchain yaratib bo'lmadi!");
    }
```

- **`preTransform`**: rasmni burish yoki aylantirish (masalan, telefonda ekran 90° burilganda). `currentTransform` "hech narsani o'zgartirma" degani.
- **`compositeAlpha`**: oyna orqasidagi narsalar ko'rinishi kerakmi? `OPAQUE` bo'lsa, oyna shaffof bo'lmaydi.
- **`clipped = VK_TRUE`**: oynaning boshqa oyna bilan to'silgan qismidagi piksellar hisoblanmaydi.
- **`oldSwapchain`**: oyna o'lchami o'zgarganda swapchain qaytadan yaratiladi va eskisi shu yerga beriladi. Hozircha oyna o'lchami o'zgarmaydi (`GLFW_RESIZABLE = FALSE`).

---

## 7-qadam: rasmlarni olish

```cpp
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}
```

- `minImageCount` faqat **minimal** son. Drayver ko'proq rasm yaratishi mumkin, shuning uchun haqiqiy sonni yana "ikki marta chaqirish" uslubi bilan so'raymiz.
- Format va o'lchamni saqlab qo'yamiz, chunki ular keyingi darslarda (image view va pipeline) kerak bo'ladi.
- **`VkImage` larni o'chirmaymiz.** Ularni swapchain yaratgan va swapchain o'chirilganda ular ham avtomatik o'chadi.

---

## 8-qadam: tozalash

Swapchain **device**'ga tegishli: u device orqali yaratilgan. Shuning uchun u device'dan **oldin** o'chiriladi:

```cpp
void HelloTriangleApplication::cleanup() {
    vkDestroySwapchainKHR(device, swapChain, nullptr);     // YANGI
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    // ...
}
```

---

## 💡 Maslahat: Vulkan qiymatlarini matnga aylantirish

`VkFormat` va boshqa enum qiymatlari oddiy son. Masalan, `VK_FORMAT_B8G8R8A8_SRGB` aslida `50`. Ularni o'qiladigan matnga aylantirish uchun Vulkan'da tayyor yordamchi funksiyalar bor:

```cpp
#include <vulkan/vk_enum_string_helper.h>

std::cout << string_VkFormat(surfaceFormat.format) << std::endl;      // VK_FORMAT_B8G8R8A8_SRGB
std::cout << string_VkPresentModeKHR(presentMode) << std::endl;       // VK_PRESENT_MODE_MAILBOX_KHR
std::cout << string_VkColorSpaceKHR(surfaceFormat.colorSpace) << std::endl;
```

Darsning boshidagi natija qatori shu funksiyalar bilan chiqarilgan.

---

## Kutilgan natija

```
----[ INFO ]-----Window surface yaratildi!
...
----[ INFO ]-----Logical device yaratildi!
----[ INFO ]-----Swapchain yaratildi: 3 ta rasm, 800x600, VK_FORMAT_B8G8R8A8_SRGB, VK_PRESENT_MODE_MAILBOX_KHR
```

Oyna avvalgidek **bo'sh** qoladi. Swapchain rasmlari tayyor, lekin ularga hali hech narsa chizilmagan.

---

## ✅ Sinov: `vkDestroySwapchainKHR` ni unutib ko'ring

```
----[ ERROR ]-----vkDestroyDevice(): VkDevice 0x... has 1 leaked objects that have not been destroyed.
VkSwapchainKHR 0x...
The Vulkan spec states: All child objects created on device that can be destroyed or freed must have been
destroyed or freed prior to destroying device (...#VUID-vkDestroyDevice-device-05137)
```

Bu safar xato `vkDestroyInstance` da emas, **`vkDestroyDevice`** da chiqadi, chunki swapchain device'ning "farzandi".

---

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| `'clamp' is not a member of 'std'` | `#include <algorithm>` yo'q |
| `'numeric_limits' is not a member of 'std'` | `#include <limits>` yo'q |
| `Mos GPU topilmadi!` | `checkDeviceExtensionSupport` noto'g'ri qiymat qaytaryapti |
| `vkCreateSwapchainKHR` da validation xatosi yoki dastur qulaydi | `VK_KHR_swapchain` logical device'da **yoqilmagan**: `getRequiredDeviceExtensions` `requiredDeviceExtensions` dan boshlanmagan |
| Ba'zan ishlaydi, ba'zan yo'q | `familyIndices` `if` ichida e'lon qilingan |

---

## Keyingi qadam

Swapchain rasmlarini (`VkImage`) to'g'ridan-to'g'ri ishlatib bo'lmaydi. Ular uchun **image view** kerak bo'ladi. Image view rasmning qaysi qismini va qanday "ko'rish" kerakligini aytadi. Bu 7-darsda.

---

## 🧩 Mashqlar

### 1. Qo'llab-quvvatlanadigan formatlar va mode'larni chiqaring

`utils` ga `printSwapChainSupport(const SwapChainSupportDetails&)` funksiyasini yozing. U barcha formatlarni, rang fazolarini va present mode'larni `string_Vk...` funksiyalari yordamida chiqarsin. Natija 3- va 4-qadamlardagi ro'yxatlarga mos kelishi kerak.

Yordam: `utils.h` `SwapChainSupportDetails` ni bilishi uchun bu struct'ni alohida header'ga (masalan, `vulkan_types.h`) ko'chirishingiz kerak bo'ladi. Bitta struct'ni ikki faylda e'lon qilib bo'lmaydi.

### 2. Imkoniyatlarni chiqaring

`capabilities` dagi `minImageCount`, `maxImageCount`, `currentExtent`, `minImageExtent` va `maxImageExtent` ni chiqaring. Sizda quyidagi natija chiqishi kerak:

```
minImageCount=2 maxImageCount=8 currentExtent=800x600 min=800x600 max=800x600
```

`min` va `max` nima uchun bir xil chiqdi, deb o'ylaysiz? Yordam: `initWindow` dagi `GLFW_RESIZABLE` ga qarang.

### 3. `+1` ni olib tashlang *(kuzatish)*

`imageCount = minImageCount` qilib ko'ring. Drayver nechta rasm yaratadi? Natijani `swapChainImages.size()` orqali tekshiring. Keyin `+1` ni qaytarib qo'ying.
