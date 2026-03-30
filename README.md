# StructsBuilder

🌐 **English** | [简体中文](README.zh-CN.md) | [繁體中文](README.zh-TW.md) | [日本語](README.ja.md)

A lightweight, header-only C++ library for **dynamically building struct memory layouts at runtime**. Push fields of arbitrary types, and read back a contiguous memory block (`void*`) that mirrors a real C/C++ struct — complete with proper byte alignment.

---

## 📦 Features

- **Header-only** — just `#include "StructsBuilder.h"`, no linking required
- **Type-agnostic fields** — push `int`, `double`, custom POD types, etc. into a single builder
- **Flexible insertion** — append, prepend, or insert at any position
- **Stream operator** — chainable `<<` syntax for quick assembly
- **Byte alignment control** — default (natural) alignment, or specify a custom packing
- **Batch push** — pass an array pointer + size to push multiple elements at once
- **RAII cleanup** — memory is released automatically on destruction

---

## 🚀 Quick Start

```cpp
#include "StructsBuilder.h"
#include <cstdio>

// Target struct layout
struct MyStruct {
    int    id;
    double value;
    char   flag;
};

int main() {
    StructsBuilder builder;

    int    id    = 42;
    double value = 3.14;
    char   flag  = 'A';

    // Build the struct field by field
    builder.push_back(id);
    builder.push_back(value);
    builder.push_back(flag);

    // Read out the packed memory
    void* data = nullptr;
    std::size_t size = builder.read_data(data);

    // Cast and use
    MyStruct* s = reinterpret_cast<MyStruct*>(data);
    printf("id=%d, value=%.2f, flag=%c\n", s->id, s->value, s->flag);
    // Output: id=42, value=3.14, flag=A

    free(data);
    return 0;
}
```

---

## 📖 API Reference

### Construction & Destruction

| Method | Description |
|---|---|
| `StructsBuilder()` | Default constructor — creates an empty builder |
| `~StructsBuilder()` | Destructor — frees all internally allocated memory |

### Adding Fields

| Method | Description |
|---|---|
| `push_back(const T& data)` | Append a single field to the end |
| `push_back(const T* data, size_t size)` | Append an array of fields to the end |
| `push_front(const T& data)` | Prepend a single field to the front |
| `push_front(const T* data, size_t size)` | Prepend an array of fields to the front |
| `insert(size_t pos, const T& data)` | Insert a single field at position `pos` (0-based) |
| `insert(size_t pos, const T* data, size_t size)` | Insert an array of fields starting at position `pos` |
| `operator<<(const T& data)` | Stream-style alias for `push_back` |

> All insertion methods return `StructsBuilder&`, enabling method chaining:
> ```cpp
> builder.push_back(x).push_back(y).push_back(z);
> // or
> builder << x << y << z;
> ```

### Reading & Memory

| Method | Description |
|---|---|
| `read_data(void*& data, size_t alignment = 0)` | Build the struct into a contiguous memory block. Returns the total byte size. Pass `0` (default) for natural alignment, or a custom value (e.g., `1`, `4`, `8`) for explicit packing. |
| `empty()` | Returns `true` if no fields have been pushed |
| `clear()` | Removes all fields and frees internal memory |

> **Important:** The `void*` returned by `read_data` is allocated with `malloc`. The caller is responsible for calling `free()` on it when done.

---

## 🔧 Byte Alignment

`read_data` supports two alignment modes:

### Default Alignment (`byte_alignment = 0`)

Mirrors the compiler's natural struct packing rules:
- Each field is aligned to a boundary equal to its own size
- The total struct size is padded to a multiple of the largest field

```
Example: int (4B) + double (8B) + char (1B)

Offset 0:  [int   ][pad   ]      — 4 bytes data + 4 bytes padding
Offset 8:  [   double     ]      — 8 bytes, naturally aligned
Offset 16: [c][  padding  ]      — 1 byte + 7 bytes tail padding
Total: 24 bytes
```

### Custom Alignment (`byte_alignment = N`)

Each field's effective alignment is `min(field_size, N)`:

```
Example with byte_alignment = 4: int (4B) + double (8B) + char (1B)

Offset 0:  [int   ]              — aligned to min(4,4) = 4
Offset 4:  [   double     ]      — aligned to min(8,4) = 4
Offset 12: [c][pad]              — aligned to min(1,4) = 1, tail pad to 4
Total: 16 bytes
```

---

## ⚠️ Limitations

| Limitation | Details |
|---|---|
| **Not thread-safe** | Do not share a single `StructsBuilder` instance across threads without external synchronization |
| **POD types only** | Fields must be trivially copyable. Types with non-trivial constructors/destructors are not supported |
| **No array types via `operator<<`** | Use the `push_back(ptr, size)` overload instead — `operator<<` with an array will cause a compilation error |
| **Manual memory management** | The `void*` from `read_data` must be `free()`-d by the caller |

---

## 📁 Project Structure

```
StructsBuilder/
├── StructsBuilder.h   # The entire library (header-only)
├── LICENSE            # Apache License 2.0
├── README.md          # English documentation
├── README.zh-CN.md    # 简体中文文档
├── README.zh-TW.md    # 繁體中文文件
└── README.ja.md       # 日本語ドキュメント
```

---

*Created by Sway, November 2021.*
