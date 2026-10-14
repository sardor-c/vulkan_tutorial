# 5-dars: Window surface va present queue

**Natija:** Vulkan GLFW oynasiga ulanadi va rasmni ekranga chiqaradigan navbat (present queue) olinadi. Konsolga `Window surface yaratildi!` chiqadi.

## Surface nima?

Vulkan **platformadan mustaqil**: u Windows, macOS yoki Linux oynalari haqida hech narsa bilmaydi. Oynaga rasm chiqarish uchun Vulkan bilan oyna o'rtasida **ko'prik** kerak. Bu ko'prik **surface** (`VkSurfaceKHR`) deyiladi.

```
Vulkan  ──►  VkSurfaceKHR  ──►  Windows oynasi (HWND)
                              └►  macOS oynasi (CAMetalLayer)
```

Surface yaratish har bir platformada boshqacha:
- Windows'da `vkCreateWin32SurfaceKHR` ishlatiladi;
- macOS'da `vkCreateMetalSurfaceEXT` ishlatiladi.

GLFW bularning hammasini bitta `glfwCreateWindowSurface` funksiyasiga yashiradi. 1-darsda GLFW nima uchun `VK_KHR_surface` va `VK_KHR_win32_surface` extension'larini so'ragani endi tushunarli: ular aynan shu ish uchun kerak edi.

## Present nima?

**Present** tayyor rasmni oynaga **chiqarish** degani. Bu ham GPU navbatiga yuboriladigan ish. Lekin har qanday navbat oila buni qila olmaydi.

Present qilish imkoniyati **oila va surface juftligiga** bog'liq. Masalan, ikkita monitor ikki xil GPU'ga ulangan bo'lsa, bitta GPU ikkinchi monitordagi oynaga chiqara olmaydi. Shuning uchun:
1. avval surface yaratiladi;
2. keyin GPU tanlanadi va har bir oiladan "shu surface'ga chiqara olasanmi?" deb so'raladi.

Sizning RTX 3060 dagi natija (sinovdan olingan):

| Oila | Imkoniyatlar | Present |
|---|---|---|
| 0 | GRAPHICS COMPUTE TRANSFER | ✅ ha |
| 1 | TRANSFER | ❌ |
| 2 | COMPUTE TRANSFER | ✅ ha |
| 3 | TRANSFER, VIDEO_DECODE | ❌ |
| 4 | TRANSFER, VIDEO_ENCODE | ❌ |
| 5 | TRANSFER | ❌ |

Ko'pchilik GPU'larda GRAPHICS oilasi present ham qila oladi, lekin bu **kafolatlanmagan**. Shuning uchun present oilasini alohida qidiramiz.

---

## 1-qadam: header

`QueueFamilyIndices` ga yangi oila:

```cpp
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    std::optional<uint32_t> presentFamily;          // YANGI

    // ... hasGraphicsFamily(), hasComputeFamily() ...

    bool hasPresentFamily() const {                  // YANGI
        return presentFamily.has_value();
    }
};
```

Class a'zolariga (`debugMessenger` dan keyin):

```cpp
    VkSurfaceKHR surface = VK_NULL_HANDLE;           // YANGI
    // ...
    VkQueue presentQueue = VK_NULL_HANDLE;           // YANGI
```

Funksiyalarga:

```cpp
    void createSurface();
```

---

## 2-qadam: surface yaratish

```cpp
void HelloTriangleApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();            // YANGI: GPU tanlashdan OLDIN
    pickPhysicalDevice();
    createLogicalDevice();
}
```

```cpp
void HelloTriangleApplication::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Window surface yaratib bo'lmadi!");
    }
}
```

- Argumentlar: instance, GLFW oynasi, allocator (`nullptr`) va natija yoziladigan joy. Bu `vkCreate...` funksiyalariga juda o'xshaydi, chunki ichida aynan shunday funksiya chaqiriladi.
- **Nima uchun `pickPhysicalDevice` dan oldin?** 3-qadamda GPU'ni tanlayotganda "bu oila **shu surface**'ga chiqara oladimi?" deb so'raymiz. Buning uchun surface allaqachon mavjud bo'lishi kerak.
- 0-darsdagi `GLFW_NO_API` bu yerda muhim: OpenGL konteksti bor oynaga Vulkan surface ulab bo'lmaydi.

---

## 3-qadam: tozalash

Surface `vkCreate...` bilan yaratilgan (GLFW orqali bo'lsa ham), shuning uchun uni o'chirish kerak. U **instance**'ga tegishli, shuning uchun funksiya `instance` ni oladi:

```cpp
void HelloTriangleApplication::cleanup() {
    vkDestroyDevice(device, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);     // YANGI

    if (enableValidationLayers) {
        utils::DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}
```

GLFW'da `glfwDestroyWindowSurface` funksiyasi **yo'q**, shuning uchun surface Vulkan'ning o'z funksiyasi bilan o'chiriladi.

```
yaratish:   oyna → instance → messenger → surface → (GPU tanlash) → device
o'chirish:  device → surface → messenger → instance → oyna
```

---

## 4-qadam: present oilasini topish

`findQueueFamilies` siklida, compute tekshiruvidan keyin:

```cpp
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport && !indices.presentFamily.has_value()) {
            indices.presentFamily = i;
        }

        if (indices.hasGraphicsFamily() && indices.hasComputeFamily() && indices.hasPresentFamily()) {
            break;
        }
```

- **`vkGetPhysicalDeviceSurfaceSupportKHR`** "`device` GPU'sining `i`-oilasi `surface` ga chiqara oladimi?" deb so'raydi. Javob `presentSupport` ga yoziladi.
- **`VkBool32`** Vulkan'ning mantiqiy turi. Bu C++'ning `bool` i emas, 32 bitli son: `VK_TRUE` (1) yoki `VK_FALSE` (0). Vulkan C kutubxonasi bo'lgani uchun o'z turidan foydalanadi.
- `break` sharti endi uchta oilani tekshiradi.

---

## 5-qadam: GPU tanlash sharti

Oynaga chiqara olmaydigan GPU bizga kerak emas. `rateGpu` boshidagi shartni o'zgartiring:

```cpp
    if (!indices.hasGraphicsFamily() || !indices.hasPresentFamily()) {
        return 0;
    }
```

`||` mantiqiy YOKI: ikkala oiladan birortasi bo'lmasa ham, GPU mos emas.

---

## 6-qadam: noyob oilalar uchun navbatlar (`std::set`)

Endi uchta oila bor va ular bir-biriga teng bo'lishi mumkin. Sizda `graphics=0`, `compute=2`, `present=0`. 4-darsdagidek `if` lar bilan barcha holatlarni tekshirish chalkash bo'lib ketadi. Buning o'rniga **`std::set`** ishlatamiz.

Fayl boshiga:

```cpp
#include <set>
```

`createLogicalDevice` dagi `graphicQueueCreateInfo`, `computeQueueCreateInfo` va ularni vector'ga yig'adigan `if` ning **hammasi** shu kod bilan almashtiriladi:

```cpp
void HelloTriangleApplication::createLogicalDevice() {
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    std::set<uint32_t> uniqueQueueFamilies = {
        indices.graphicsFamily.value(),
        indices.computeFamily.value(),
        indices.presentFamily.value()
    };

    float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t family : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = family;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    auto deviceExtensions = getRequiredDeviceExtensions(physicalDevice);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    // ... qolgani avvalgidek ...
```

### `std::set` nima?

`std::set` **takrorlanmaydigan** elementlar to'plami. Bir xil qiymatni ikki marta qo'shsangiz, u bir marta saqlanadi:

```cpp
std::set<uint32_t> s = { 0, 2, 0 };   // ichida: { 0, 2 }
s.size();                             // 2
```

Sizda `{0, 2, 0}` → `{0, 2}`, ya'ni ikkita `VkDeviceQueueCreateInfo` hosil bo'ladi. Faqat bitta GRAPHICS oilasi bor GPU'da esa `{0, 0, 0}` → `{0}` bo'lib, bittasi hosil bo'ladi. 4-darsdagi "bir oila ikki marta so'raldi" xatosi endi **umuman yuz bermaydi**.

### ⚠️ `queuePriority` sikldan **tashqarida**

Uni sikl ichida e'lon qilsangiz, har aylanish oxirida u o'chiriladi, `pQueuePriorities` esa yo'q bo'lgan o'zgaruvchiga ishora qilib qoladi. Bu 4-darsdagi vector bilan bir xil xato. `queuePriority` ning `vkCreateDevice` gacha "tirik" bo'lishi shart.

`queueCreateInfo` ni esa sikl ichida e'lon qilish mumkin, chunki `push_back` uni vector ichiga **nusxalaydi**.

---

## 7-qadam: navbatlarni olish

`vkCreateDevice` dan keyin uchala navbatni ham **shartsiz** oling:

```cpp
    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.computeFamily.value(), 0, &computeQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
```

Sizda graphics va present bir xil oiladan (0) olinadi, shuning uchun `graphicsQueue == presentQueue` bo'ladi. Bu to'g'ri: bitta navbat ham chizadi, ham ekranga chiqaradi.

---

## Kutilgan natija

```
----[ INFO ]-----Validation layer yoqilgan!
----[ INFO ]-----Barcha kerakli extensionlar topildi!
----[ INFO ]-----VkInstance muvaffaqiyatli yaratildi!
----[ INFO ]-----Window surface yaratildi!
----[ DEBUG ]----Tanlangan GPU: NVIDIA GeForce RTX 3060 (rate: 132768 )
...
----[ INFO ]-----Logical device yaratildi!
```

---

## ✅ Sinov: `vkDestroySurfaceKHR` ni unutib ko'ring

```
----[ ERROR ]-----vkDestroyInstance(): VkInstance 0x... has 1 leaked objects that have not been destroyed.
VkSurfaceKHR 0x...
The Vulkan spec states: All child objects that were created with instance ... must have been destroyed
or freed prior to destroying instance (...#VUID-vkDestroyInstance-instance-00629)
```

Bu yana o'sha tanish xato, bu safar "yetim" qolgan obyekt `VkSurfaceKHR`.

---

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| `'set' is not a member of 'std'` | `#include <set>` yo'q |
| `Window surface yaratib bo'lmadi!` | Oyna `GLFW_NO_API` siz yaratilgan. Bunday holatda GLFW `VK_ERROR_NATIVE_WINDOW_IN_USE_KHR` qaytaradi |
| `vkGetPhysicalDeviceSurfaceSupportKHR` da surface haqida validation xatosi | `createSurface()` `pickPhysicalDevice()` dan **keyin** chaqirilgan, ya'ni GPU tanlanayotganda `surface` hali `VK_NULL_HANDLE` edi |
| `Mos GPU topilmadi!` | `presentFamily` topilmagan. `vkGetPhysicalDeviceSurfaceSupportKHR` ga to'g'ri `i` va `surface` berilganini tekshiring |
| Ba'zan ishlaydi, ba'zan qulaydi | `queuePriority` sikl ichida e'lon qilingan |

---

## macOS haqida

Bu darsda macOS uchun alohida ish yo'q. GLFW macOS'da `VK_EXT_metal_surface` extension'ini o'zi so'raydi va u `getRequiredExtensions()` orqali avtomatik qo'shiladi.

---

## 🧩 Mashqlar

### 1. Present qila oladigan oilalarni chiqaring

`utils::printDeviceQueueFamilies` ga `VkSurfaceKHR surface` parametrini qo'shing va har bir oila yonida `PRESENT` so'zini chiqaring. Natija darsning boshidagi jadvalga mos kelishi kerak: 0- va 2-oilalarda `PRESENT` chiqadi.

### 2. Graphics va present uchun bitta oila

Graphics va present **bitta oiladan** bo'lsa, ish tezroq bo'ladi: rasm bir navbatdan ikkinchisiga "uzatilmaydi". Hozirgi kod birinchi topilgan oilani oladi. Uni shunday o'zgartiring: avval **ikkalasini ham** qila oladigan oila qidirilsin, faqat u topilmasagina alohida oilalar olinsin.

Yordam: siklda `(flags & VK_QUEUE_GRAPHICS_BIT) && presentSupport` shartini tekshiring. Sizning GPU'da natija o'zgarmaydi (0-oila), lekin boshqa GPU'larda farq qilishi mumkin.

### 3. `findQueueFamilies` ni bir marta chaqiring *(fikrlash)*

Hozir `findQueueFamilies` ikki marta chaqiriladi: `rateGpu` ichida (har bir GPU uchun) va `createLogicalDevice` ichida. Mashq 1 dagi kabi ichiga `std::cout` qo'shsangiz, buni o'zingiz ko'rasiz.

Tanlangan GPU'ning oilalarini class a'zosiga (`QueueFamilyIndices queueFamilyIndices;`) saqlab, `createLogicalDevice` da undan foydalaning. Bu natijalar keyingi darslarda (swapchain, command pool) ham kerak bo'ladi.
