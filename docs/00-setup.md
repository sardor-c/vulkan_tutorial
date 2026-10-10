# 0-dars: Muhitni sozlash

**Natija:** oyna ochiladi, konsolga Vulkan extension'lar soni va GLM testi chiqadi.

## Qaysi vosita nima qiladi

| Vosita | Vazifasi |
|---|---|
| **g++** | C++ kodni `.exe` ga aylantiradigan kompilyator (MSYS2 mingw64) |
| **CMake** | Loyiha "retsepti": qaysi fayllar, qaysi kutubxonalar bilan |
| **Ninja** | CMake yozgan retsept bo'yicha g++ ni tez chaqiradi |
| **Vulkan** | GPU bilan gaplashish uchun API |
| **GLFW** | Windows oynasini ochadi. Vulkan o'zi oyna ocha olmaydi |
| **GLM** | Grafika matematikasi: vektorlar va matritsalar |

Ish tartibi: **siz kod yozasiz → CMake retsept tayyorlaydi → Ninja g++ ni chaqiradi → `.exe`**.

### O'rnatilgan paketlar (MSYS2)

```
pacman -S mingw-w64-x86_64-vulkan-devel mingw-w64-x86_64-shaderc \
          mingw-w64-x86_64-glfw mingw-w64-x86_64-glm \
          mingw-w64-x86_64-cmake mingw-w64-x86_64-ninja
```

Windows'da foydalanuvchi uchun `VK_ADD_LAYER_PATH=C:\msys64\mingw64\bin` o'zgaruvchisi o'rnatilgan. Busiz Vulkan validation layer'ni topa olmaydi (2-darsga qarang).

## Papka tuzilishi

```
vulkan_tutorial/
├── CMakeLists.txt
├── src/
│   └── main.cpp
└── build/          ← CMake o'zi yaratadi
```

## CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)
project(vulkan_tutorial CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

add_executable(vulkan_tutorial src/main.cpp)
target_link_libraries(vulkan_tutorial PRIVATE Vulkan::Vulkan glfw glm::glm)
```

| Qator | Ma'nosi |
|---|---|
| `cmake_minimum_required` | CMake'ning eng past talab qilinadigan versiyasi. Doim birinchi turadi |
| `project(... CXX)` | Loyiha nomi. `CXX` bu C++ degani |
| `CMAKE_CXX_STANDARD 20` | C++20 standartidan foydalanamiz. `REQUIRED ON`: kompilyator bilmasa, xato beradi |
| `find_package(X REQUIRED)` | Kutubxonani qidiradi. Topilmasa, shu yerda to'xtaydi |
| `add_executable` | Ko'rsatilgan `.cpp` fayllardan `.exe` yasaydi. Yangi `.cpp` qo'shsangiz, shu yerga yozasiz |
| `target_link_libraries` | Dasturni kutubxonalarga ulaydi. Busiz `undefined reference to vk...` xatosi chiqadi |

## main.cpp

```cpp
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan oyna", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << extensionCount << " ta extension bor" << std::endl;

    glm::mat4 matrix(1.0f);
    glm::vec4 vec(1.0f, 2.0f, 3.0f, 1.0f);
    glm::vec4 test = matrix * vec;
    std::cout << "GLM: " << test.x << ", " << test.y << ", " << test.z << std::endl;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
```

### Muhim joylar

- **`#define GLFW_INCLUDE_VULKAN`** `#include` dan **oldin** yozilishi shart. Shunda GLFW Vulkan header'ini ham ulaydi.
- **`GLM_FORCE_DEPTH_ZERO_TO_ONE`**: OpenGL'da chuqurlik −1…1 oralig'ida, Vulkan'da esa 0…1 oralig'ida.
- **`GLFW_NO_API`**: GLFW'ga OpenGL kontekst yaratmaslikni aytadi. Vulkan'da "kontekst" tushunchasi yo'q.
- **`vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr)`**: Vulkan'ning **"ikki marta chaqirish"** uslubi. Birinchi chaqiruvda faqat soni olinadi, ikkinchisida ma'lumotlarning o'zi (1-darsdagi mashqqa qarang).
- **`matrix * vec`**: tartib muhim. Avval matritsa, keyin vektor yoziladi. `vec * matrix` boshqa natija beradi.
- **Main loop**: `glfwPollEvents()` klaviatura, sichqoncha va "yopish" hodisalarini qayta ishlaydi. Keyinchalik har kadr shu siklda chiziladi.
- **Tozalash teskari tartibda**: oxirgi yaratilgan narsa birinchi o'chiriladi.

## VS Code IntelliSense

`.vscode/c_cpp_properties.json` g++ va `build/compile_commands.json` faylidan foydalanadi. `.vscode/settings.json` esa CMake'ga bu faylni yaratishni aytadi. `#include` qatorlari qizil bo'lib qolsa:

```
cmake -S . -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

so'ng `Ctrl+Shift+P` → `Developer: Reload Window`.

## Ko'p uchraydigan xatolar

| Xato | Sababi |
|---|---|
| `cmake: command not found` | Terminal eski. VS Code'ni qayta oching |
| `Could NOT find Vulkan` | `find_package` nomida xato bor yoki paket o'rnatilmagan |
| `undefined reference to vk...` | `target_link_libraries` yo'q yoki xato yozilgan |
| `glfw3.h: No such file` | `GLFW/glfw3.h`: papka nomi katta harflar bilan yoziladi |
| DLL topilmadi | PATH'da `C:\msys64\mingw64\bin` yo'q |
