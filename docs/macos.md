# macOS'da ishlatish

**Muammo:** macOS'da dastur `VkInstance yaratib bo'lmadi!` xatosi bilan to'xtaydi.

## Sababi

macOS'da Vulkan drayveri yo'q, Apple faqat o'zining **Metal** API'sini qo'llab-quvvatlaydi. Shuning uchun macOS'da **MoltenVK** ishlatiladi: u Vulkan chaqiruvlarini Metal'ga tarjima qiladi.

```
Sizning kodingiz  →  Vulkan loader  →  MoltenVK  →  Metal  →  Apple GPU
```

MoltenVK Vulkan'ning hamma talablarini to'liq bajara olmaydi. Masalan, Metal'da ba'zi Vulkan imkoniyatlarining o'xshashi yo'q. Bunday drayverlar **"portability" drayverlari** deyiladi.

**Vulkan loader 1.3.216** versiyasidan beri portability drayverlarini **sukut bo'yicha yashiradi**. Buning maqsadi dastur o'zi bilmagan holda "to'liq bo'lmagan" drayverga tushib qolmasligi. Dastur ularni ko'rish uchun aniq ruxsat so'rashi kerak. Ruxsat so'ralmasa, loader birorta ham drayver topmaydi va `vkCreateInstance` **`VK_ERROR_INCOMPATIBLE_DRIVER`** qaytaradi.

## Tuzatish

Ikkita o'zgarish kerak: **extension** va **bayroq**.

### 1. Extension qo'shish

`getRequiredExtensions()` ga:

```cpp
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
```

`VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME` bu `"VK_KHR_portability_enumeration"` matni.

### 2. Bayroq qo'shish

`createInstance()` da, `ppEnabledExtensionNames` dan keyin:

```cpp
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

#ifdef __APPLE__
    createInfo.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif
```

**Ikkalasi ham shart:**
- **extension** loader'ga "men portability haqida bilaman" deydi;
- **bayroq** esa "portability drayverlarini ham ro'yxatga qo'sh" deydi.

`|=` belgisi `flags = flags | BIT` ning qisqa yozuvi: mavjud bayroqlarni saqlab, yangisini qo'shadi.

### `#ifdef __APPLE__` nima?

`__APPLE__` makrosini kompilyator **Apple platformalarida** (macOS va iOS) avtomatik e'lon qiladi. Shuning uchun:
- macOS'da bu qatorlar kompilyatsiya qilinadi;
- Windows'da ular umuman yo'qdek bo'ladi va kodingiz avvalgidek ishlaydi.

2-darsdagi `#ifdef NDEBUG` ham xuddi shu tarzda ishlaydi.

> **Muqobil yo'l.** `#ifdef` o'rniga avval extension mavjudligini tekshirib, mavjud bo'lsa yoqish mumkin. Bu usul har qanday platformada ishlaydi. Windows'da ham `VK_KHR_portability_enumeration` mavjud va uni yoqish zarar qilmaydi: bu Windows'da sinab ko'rilgan. Lekin `#ifdef __APPLE__` soddaroq va tushunarliroq.

`checkRequiredExtensions()` funksiyangiz macOS'da yangi extension'ni ham avtomatik tekshiradi, chunki u `getRequiredExtensions()` qaytargan butun ro'yxatni ko'rib chiqadi.

---

## 💡 Maslahat: xato kodini ko'rsatish

Hozir `vkCreateInstance` muvaffaqiyatsiz bo'lsa, faqat "yaratib bo'lmadi" degan xabar chiqadi, **nima uchun** ekani ko'rinmaydi. `VkResult` ni matnga aylantirsangiz, sababi darhol ko'rinadi:

```cpp
#include <vulkan/vk_enum_string_helper.h>   // string_VkResult uchun
#include <string>
```

```cpp
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error(std::string("VkInstance yaratib bo'lmadi: ") + string_VkResult(result));
    }
```

Shunda macOS'da tuzatishsiz quyidagi xabar chiqadi:

```
VkInstance yaratib bo'lmadi: VK_ERROR_INCOMPATIBLE_DRIVER
```

- `std::string("...") + ...`: oddiy `const char*` matnlarni `+` bilan qo'shib bo'lmaydi. Birinchisini `std::string` ga aylantirish kerak.
- `vk_enum_string_helper.h` Vulkan SDK'da ham, MSYS2 paketlarida ham bor.

Bu usulni boshqa `vkCreate...` chaqiruvlari uchun ham ishlatish foydali.

---

## Keyinroq: logical device (4-dars)

macOS uchun yana bitta qadam **logical device yaratishda** kerak bo'ladi. GPU `VK_KHR_portability_subset` **device extension**'ini qo'llab-quvvatlasa (MoltenVK'da doim qo'llab-quvvatlaydi), uni yoqish **majburiy**. Aks holda validation layer xato beradi. Bu [4-darsda](04-logical-device.md) (4-qadam) qo'shilgan.

---

## macOS'da yig'ish

> Bu bo'lim macOS'da sinab ko'rilmagan.

1. [LunarG Vulkan SDK](https://vulkan.lunarg.com/sdk/home#mac) ni o'rnating. Unda MoltenVK, loader va validation layer'lar bor.
2. Qolgan kutubxonalar:
   ```
   brew install cmake ninja glfw glm
   ```
3. Har bir yangi terminalda (yoki `~/.zshrc` ga qo'shib qo'ying):
   ```
   source ~/VulkanSDK/<versiya>/setup-env.sh
   ```
   Bu buyruq loader MoltenVK va validation layer'ni topishi uchun kerakli yo'llarni o'rnatadi. U Windows'dagi `VK_ADD_LAYER_PATH` ning o'xshashi.
4. Build buyruqlari Windows'dagidek:
   ```
   cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
   cmake --build build
   ./build/vulkan_tutorial
   ```

`color.h` dagi ranglar macOS terminalida ham ishlaydi.
