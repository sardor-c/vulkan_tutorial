# Vulkan darslari

Har bir dars alohida faylda. Kodni o'zingiz yozasiz, darsda esa nima va nima uchun yozilishi tushuntiriladi.

| # | Dars | Natija |
|---|------|--------|
| 0 | [Muhitni sozlash](00-setup.md) | CMake loyihasi, GLFW oynasi, GLM testi |
| 1 | [VkInstance yaratish](01-instance.md) | Vulkan bilan birinchi aloqa |
| 2 | [Validation layer'lar](02-validation-layers.md) | Vulkan xatolaringizni aytib turadi |
| 3 | [GPU tanlash va queue family'lar](03-physical-device.md) | RTX 3060 tanlanadi, GRAPHICS oilasi topiladi |
| 4 | [Logical device va graphics queue](04-logical-device.md) | GPU bilan aloqa ochiladi, navbat olinadi |

### Qo'shimcha

- [macOS'da ishlatish](macos.md): MoltenVK va portability extension'i

## Yig'ish va ishga tushirish

```
cmake --build build
.\build\vulkan_tutorial.exe
```

`CMakeLists.txt` o'zgarganda avval quyidagi buyruqni ishga tushiring:

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
```

## Foydali havolalar

- [docs.vulkan.org/tutorial](https://docs.vulkan.org/tutorial/latest/00_Introduction.html): darslar shu tartibga asoslangan
- [Vulkan spetsifikatsiyasi](https://docs.vulkan.org/spec/latest/index.html): har bir funksiya va struct'ning to'liq tavsifi
