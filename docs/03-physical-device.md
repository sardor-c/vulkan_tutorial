# 3-dars: GPU tanlash va queue family'lar

**Natija:** dastur kompyuterdagi GPU'larni topadi, mosini tanlaydi va konsolga `Tanlangan GPU: NVIDIA GeForce RTX 3060` chiqaradi.

## Physical device nima?

Vulkan'da GPU bilan ishlash ikki bosqichdan iborat:

| | Nima | Qaysi darsda |
|---|---|---|
| **Physical device** (`VkPhysicalDevice`) | Haqiqiy GPU qurilmasi. Undan **so'raymiz**: nomi nima, nimalarni qila oladi? | shu dars |
| **Logical device** (`VkDevice`) | Shu GPU bilan ishlash uchun **ochilgan aloqa**. Barcha buyruqlar shu orqali yuboriladi | 4-dars |

Kompyuterda bir nechta GPU bo'lishi mumkin. Masalan, noutbukda protsessor ichidagi grafika va alohida NVIDIA karta bo'ladi. Shuning uchun avval hammasini ro'yxatga olamiz, keyin mosini tanlaymiz.

## Queue va queue family nima?

GPU'ga buyruqlar (masalan, "uchburchak chiz" yoki "xotirani nusxala") **navbatga (queue)** qo'yib yuboriladi. GPU ularni navbatdan olib, bajaradi.

Navbatlar **oilalarga (family)** bo'lingan. Har bir oila ma'lum turdagi ishlarni bajara oladi:

| Bayroq | Nima qila oladi |
|---|---|
| `GRAPHICS` | Chizish: uchburchaklar, teksturalar, piksellar |
| `COMPUTE` | Umumiy hisob-kitoblar (compute shader'lar) |
| `TRANSFER` | Xotirani bir joydan boshqasiga nusxalash |
| `VIDEO_DECODE` / `VIDEO_ENCODE` | Videoni ochish va siqish |

Sizning RTX 3060 da 6 ta oila bor (mashqdagi kod buni ko'rsatadi):

```
oila 0: 16 ta navbat | GRAPHICS COMPUTE TRANSFER SPARSE    ← bizga shu kerak
oila 1:  2 ta navbat | TRANSFER SPARSE
oila 2:  8 ta navbat | COMPUTE TRANSFER SPARSE
oila 3:  1 ta navbat | TRANSFER SPARSE VIDEO_DECODE
oila 4:  1 ta navbat | TRANSFER SPARSE VIDEO_ENCODE
oila 5:  1 ta navbat | TRANSFER SPARSE
```

Hozircha bizga **GRAPHICS** qo'llab-quvvatlaydigan bitta oila kerak. Uning **raqamini (indeksini)** topib, saqlab qo'yamiz. 4-darsda logical device yaratishda shu raqam kerak bo'ladi.

---

## 1-qadam: header

`HelloTriangleApplication.h`:

```cpp
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <optional>     // YANGI
#include <vector>

// YANGI: class'dan OLDIN
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;

    bool isComplete() const {
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication {
    // ...
private:
    GLFWwindow* window = nullptr;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;           // YANGI

    // ... avvalgi funksiyalar ...

    // YANGI
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
};
```

### `std::optional` nima?

Oila raqami `0` bo'lishi mumkin va sizda aynan shunday. Agar raqamni oddiy `uint32_t` da saqlasak, **"topilmadi"** holatini qanday ifodalaymiz? `0` raqami band, `-1` esa `uint32_t` ga sig'maydi.

`std::optional<uint32_t>` bu "qiymati bor yoki yo'q bo'lishi mumkin bo'lgan `uint32_t`":

```cpp
std::optional<uint32_t> x;      // bo'sh
x.has_value();                  // false

x = 0;                          // endi qiymati bor
x.has_value();                  // true
x.value();                      // 0
```

### `struct` va `class` farqi

Ular deyarli bir xil. Yagona farqi shundaki, `struct` ichidagi hamma narsa sukut bo'yicha `public`, `class` ichidagisi esa `private`. Odatda oddiy "ma'lumot qutisi" uchun `struct` ishlatiladi.

`isComplete()` dagi `const` "bu funksiya struct'ni o'zgartirmaydi" degani.

### Nima uchun class'dan oldin?

`findQueueFamilies` `QueueFamilyIndices` qaytaradi. Kompilyator faylni yuqoridan pastga o'qiydi, shuning uchun bu turni ishlatishdan oldin e'lon qilish kerak.

Keyingi darslarda bu struct'ga yangi oila qo'shiladi (`presentFamily`). Shuning uchun u alohida struct qilib yozilgan.

---

## 2-qadam: GPU'larni ro'yxatga olish

`HelloTriangleApplication.cpp`:

```cpp
void HelloTriangleApplication::initVulkan() {
    createInstance();
    setupDebugMessenger();
    pickPhysicalDevice();       // YANGI
}
```

```cpp
void HelloTriangleApplication::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Vulkan'ni qo'llab-quvvatlaydigan GPU topilmadi!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Mos GPU topilmadi!");
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "Tanlangan GPU: " << properties.deviceName << std::endl;
}
```

- Yana **"ikki marta chaqirish"**: avval GPU'lar soni, keyin ro'yxatning o'zi.
- `vkEnumeratePhysicalDevices` birinchi argument sifatida `instance` oladi. Instance'ning nima uchun birinchi yaratilganini endi ko'rib turibsiz.
- Ro'yxatdagi **birinchi mos** GPU'ni olamiz. Yaxshirog'ini tanlash 2-mashqda.
- **`vkGetPhysicalDeviceProperties`**: GPU nomi, turi, qo'llab-quvvatlanadigan Vulkan versiyasi va cheklovlari. Bu yerda `{}` shart emas, chunki Vulkan struct'ni to'liq o'zi to'ldiradi.

### Physical device'ni o'chirish kerakmi?

**Yo'q.** `VkPhysicalDevice` siz **yaratgan** narsa emas, siz uni faqat **so'ragansiz**. U instance bilan birga avtomatik yo'qoladi. Shuning uchun `cleanup()` ga hech narsa qo'shilmaydi.

Qoida: `vkCreate...` yoki `vkAllocate...` bilan yaratilgan narsani o'chirasiz. `vkEnumerate...` yoki `vkGet...` bilan olingan narsani o'chirmaysiz.

---

## 3-qadam: GPU bizga mosmi?

```cpp
bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device) {
    QueueFamilyIndices indices = findQueueFamilies(device);
    return indices.isComplete();
}
```

Hozircha bitta shart bor: GPU'da GRAPHICS oilasi bo'lishi kerak. Keyingi darslarda yangi shartlar qo'shiladi, masalan "oynaga chiza oladimi?" yoki "swapchain extension'i bormi?".

---

## 4-qadam: queue family'ni topish

```cpp
QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device) {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        if (indices.isComplete()) {
            break;
        }
    }

    return indices;
}
```

- Bu funksiya `VkResult` qaytarmaydi, chunki xato bo'la olmaydi. Shuning uchun tekshiruv kerak emas.
- **`i` indeksi muhim**, chunki oila raqami uning ro'yxatdagi o'rni. Shuning uchun `for (const auto& f : ...)` emas, oddiy `for (uint32_t i = 0; ...)` sikli ishlatilgan.

### `&` bilan bayroqni tekshirish

`queueFlags` bitta son, lekin uning har bir **biti** alohida bayroq:

```
queueFlags (oila 0)  = 0b...01111   (GRAPHICS | COMPUTE | TRANSFER | SPARSE)
VK_QUEUE_GRAPHICS_BIT = 0b...00001
                        ----------- &  (bitli VA)
natija               = 0b...00001   → 0 emas → true ✅

queueFlags (oila 2)  = 0b...01110   (COMPUTE | TRANSFER | SPARSE)
VK_QUEUE_GRAPHICS_BIT = 0b...00001
                        ----------- &
natija               = 0b...00000   → 0 → false ❌
```

2-darsda bayroqlarni `|` bilan **birlashtirgan** edingiz. Bu yerda esa `&` bilan bitta bayroq bor-yo'qligini **tekshiryapmiz**.

`if` ichida `0` qiymati `false` deb, boshqa har qanday son `true` deb hisoblanadi.

---

## Kutilgan natija

```
----[DEBUG]---- Validation layer yoqilgan!
----[DEBUG]---- Barcha kerakli extensionlar topildi!
----[DEBUG]---- VkInstance muvaffaqiyatli yaratildi!
Tanlangan GPU: NVIDIA GeForce RTX 3060
```

---

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| `'QueueFamilyIndices' does not name a type` | Struct class'dan **keyin** yozilgan yoki umuman yozilmagan |
| `'optional' in namespace 'std' does not name a template type` | `#include <optional>` yo'q |
| `Mos GPU topilmadi!` | `graphicsFamily` ga qiymat berilmagan yoki `isComplete()` noto'g'ri yozilgan |
| Xato yo'q, lekin GRAPHICS'siz oila ham tanlanadi | `&` o'rniga `&&` yozilgan. `flags && BIT` har qanday nol bo'lmagan `flags` uchun `true` bo'ladi. Bu **yashirin xato**: kompilyator ham, validation layer ham buni ko'rsatmaydi |

---

## 🧩 Mashqlar

### 1. Barcha oilalarni chiqaring

`findQueueFamilies` siklida har bir oila haqida ma'lumot chiqaring:

```cpp
const auto& f = queueFamilies[i];
std::cout << "  oila " << i << ": " << f.queueCount << " ta navbat |"
          << ((f.queueFlags & VK_QUEUE_GRAPHICS_BIT) ? " GRAPHICS" : "")
          << ((f.queueFlags & VK_QUEUE_COMPUTE_BIT)  ? " COMPUTE"  : "")
          << ((f.queueFlags & VK_QUEUE_TRANSFER_BIT) ? " TRANSFER" : "")
          << std::endl;
```

Bu yerda `shart ? A : B` **ternar operator** ishlatilgan: shart rost bo'lsa `A`, aks holda `B` qaytaradi.

Natija darsning boshidagi jadvalga o'xshashi kerak. Diqqat qiling: `break` tufayli sikl 0-oiladan keyin to'xtaydi. Barcha oilalarni ko'rish uchun `break` ni vaqtincha olib tashlang.

### 2. Eng yaxshi GPU'ni tanlang

Noutbukda birinchi topilgan GPU protsessor ichidagi kuchsiz grafika bo'lishi mumkin. GPU'larga ball bering va eng yuqori ballisini tanlang:

```cpp
VkPhysicalDeviceProperties properties;
vkGetPhysicalDeviceProperties(device, &properties);

int score = 0;
if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    score += 100000;   // alohida videokarta
}
score += properties.limits.maxImageDimension2D;   // katta tekstura = kuchliroq
```

**Diqqat:** alohida videokarta uchun beriladigan bonus `maxImageDimension2D` ning eng katta qiymatidan **katta** bo'lishi kerak. Sizning RTX 3060 da bu qiymat `32768`. Bonus, masalan, `1000` yoki `10000` bo'lsa, katta teksturani qo'llab-quvvatlaydigan ichki (integrated) GPU alohida kartadan yuqori ball olib qolishi mumkin. Shuning uchun bonus `100000`.

Yordam: `int rateDevice(VkPhysicalDevice device)` funksiyasini yozing. Mos bo'lmagan GPU uchun u `0` qaytarsin. `pickPhysicalDevice` da esa eng katta ballni saqlab boring.

### 3. Video xotira hajmini chiqaring *(qiyinroq)*

```cpp
VkPhysicalDeviceMemoryProperties memProps;
vkGetPhysicalDeviceMemoryProperties(device, &memProps);

for (uint32_t i = 0; i < memProps.memoryHeapCount; i++) {
    if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
        // memProps.memoryHeaps[i].size baytlarda. GB ga aylantiring
    }
}
```

`DEVICE_LOCAL` GPU'ning o'z xotirasi (VRAM) degani. Sizning RTX 3060 da **≈11.83 GB** chiqishi kerak. Bu 12 GB, faqat 1024 ga bo'lib hisoblangan. GPU'da yana bitta, taxminan 16 GB'lik heap ham bor. U `DEVICE_LOCAL` emas: bu kompyuterning operativ xotirasi (RAM), GPU uni ham ishlata oladi.

Maslahat: `size` turi `VkDeviceSize` (`uint64_t`). GB ga aylantirish uchun uni `1024.0 * 1024 * 1024` ga bo'ling. `.0` bo'lishning kasr son bilan bajarilishini ta'minlaydi.
