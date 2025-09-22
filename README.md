# JsonDiffPatch C++

A lightweight C++ implementation of [jsondiffpatch](https://github.com/benjamine/jsondiffpatch) with a clean C API.

This repository contains **only the C++ library**.  

---

## ✨ Features

- JSON **diff**, **patch**, and **unpatch** similar to jsondiffpatch.
- Supports object, array, and text diffing (basic).
- Move detection and efficient array diffing.
- Header-only dependency on [nlohmann/json](https://github.com/nlohmann/json) (bundled in `thirdparty/`).
- Clean **C API** (`JDP_Diff`, `JDP_Patch`, `JDP_Unpatch`, `JDP_FreeString`) for easy integration into any language.

---

## 📂 Repository Layout

```
include/        Public header files (JsonDiffPatch.h)
src/            Implementation (JsonDiffPatch.cpp)
examples/       Minimal console example
thirdparty/     Bundled nlohmann/json single-header
```

---

## 📦 Getting Started

### Clone

```bash
git clone https://github.com/YOURNAME/jsondiffpatch-cpp.git
cd jsondiffpatch-cpp
```

### Build with CMake (example)

```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

This produces `libJsonDiffPatch.a` or `JsonDiffPatch.dll` depending on your platform.

### Include in your project

- Add `include/` to your include path.
- Link to the built library (or compile `src/JsonDiffPatch.cpp` directly into your project).
- Include the header:

```cpp
#include <JsonDiffPatch/JsonDiffPatch.h>
```

---

## 🚀 How to Use

You can use the library in two ways:

### 1. Direct C++ API

```cpp
#include <JsonDiffPatch/JsonDiffPatch.h>
#include <iostream>

int main() {
    JsonDiffPatch::JsonDiffPatch jdp;

    nlohmann::json left = { {"x",1}, {"y",2} };
    nlohmann::json right = { {"x",1}, {"y",3} };

    nlohmann::json diff = jdp.Diff(left, right);
    std::cout << "Diff: " << diff.dump() << "\n";

    nlohmann::json patched = jdp.Patch(left, diff);
    std::cout << "Patched: " << patched.dump() << "\n";
}
```

### 2. C API (good for DLLs / foreign languages)

```cpp
#include <JsonDiffPatch/JsonDiffPatch.h>
#include <cstdio>

int main() {
    const char* a = "{\"x\":1,\"y\":2}";
    const char* b = "{\"x\":1,\"y\":3}";

    const char* diff = JDP_Diff(a, b);
    printf("Diff: %s\n", diff);

    const char* patched = JDP_Patch(a, diff);
    printf("Patched: %s\n", patched);

    // Free the memory allocated by the library
    JDP_FreeString(diff);
    JDP_FreeString(patched);

    return 0;
}
```

That’s it — the library will give you JSON patches that are compatible with jsondiffpatch format.

---

## 📄 License

MIT (or zlib) — do whatever you want but keep the copyright notice.
