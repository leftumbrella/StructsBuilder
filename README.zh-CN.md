# StructsBuilder

🌐 [English](README.md) | **简体中文** | [繁體中文](README.zh-TW.md) | [日本語](README.ja.md)

一个轻量级、仅头文件的 C++ 库，用于**在运行时动态构建结构体内存布局**。将任意类型的字段逐个推入构建器，即可读取一块连续内存（`void*`），其布局与真实的 C/C++ 结构体完全一致——包括正确的字节对齐。

---

## 📦 特性

- **仅头文件** — 只需 `#include "StructsBuilder.h"`，无需链接额外库
- **类型无关** — 可推入 `int`、`double`、自定义 POD 类型等任意字段
- **灵活插入** — 支持尾部追加、头部前插、任意位置插入
- **流式操作符** — 可链式调用的 `<<` 语法，快速组装结构体
- **字节对齐控制** — 支持默认（自然）对齐，也可指定自定义对齐方式
- **批量推入** — 传入数组指针 + 大小，一次性推入多个元素
- **RAII 自动清理** — 析构时自动释放所有内存

---

## 🚀 快速开始

```cpp
#include "StructsBuilder.h"
#include <cstdio>

// 目标结构体布局
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

    // 逐字段构建结构体
    builder.push_back(id);
    builder.push_back(value);
    builder.push_back(flag);

    // 读取构建好的内存
    void* data = nullptr;
    std::size_t size = builder.read_data(data);

    // 强制转换并使用
    MyStruct* s = reinterpret_cast<MyStruct*>(data);
    printf("id=%d, value=%.2f, flag=%c\n", s->id, s->value, s->flag);
    // 输出: id=42, value=3.14, flag=A

    free(data);
    return 0;
}
```

---

## 📖 API 参考

### 构造与析构

| 方法 | 说明 |
|---|---|
| `StructsBuilder()` | 默认构造函数 — 创建一个空的构建器 |
| `~StructsBuilder()` | 析构函数 — 释放所有内部分配的内存 |

### 添加字段

| 方法 | 说明 |
|---|---|
| `push_back(const T& data)` | 在末尾追加一个字段 |
| `push_back(const T* data, size_t size)` | 在末尾追加一组字段（数组） |
| `push_front(const T& data)` | 在头部前插一个字段 |
| `push_front(const T* data, size_t size)` | 在头部前插一组字段（数组） |
| `insert(size_t pos, const T& data)` | 在指定位置 `pos` 插入一个字段（从 0 开始） |
| `insert(size_t pos, const T* data, size_t size)` | 从指定位置 `pos` 开始插入一组字段 |
| `operator<<(const T& data)` | 流式操作符，等同于 `push_back` |

> 所有插入方法均返回 `StructsBuilder&`，支持链式调用：
> ```cpp
> builder.push_back(x).push_back(y).push_back(z);
> // 或者
> builder << x << y << z;
> ```

### 读取与内存

| 方法 | 说明 |
|---|---|
| `read_data(void*& data, size_t alignment = 0)` | 将结构体构建为一块连续内存。返回总字节大小。传入 `0`（默认）使用自然对齐，或传入自定义值（如 `1`、`4`、`8`）指定对齐方式。 |
| `empty()` | 如果未推入任何字段则返回 `true` |
| `clear()` | 移除所有字段并释放内部内存 |

> **重要提示：** `read_data` 返回的 `void*` 由 `malloc` 分配，调用者需要负责在使用完毕后调用 `free()` 释放。

---

## 🔧 字节对齐

`read_data` 支持两种对齐模式：

### 默认对齐（`byte_alignment = 0`）

模拟编译器的自然结构体对齐规则：
- 每个字段对齐到等于其自身大小的边界
- 结构体总大小填充为最大字段大小的倍数

```
示例: int (4B) + double (8B) + char (1B)

偏移 0:  [int   ][pad   ]      — 4 字节数据 + 4 字节填充
偏移 8:  [   double     ]      — 8 字节，自然对齐
偏移 16: [c][  padding  ]      — 1 字节 + 7 字节尾部填充
总计: 24 字节
```

### 自定义对齐（`byte_alignment = N`）

每个字段的有效对齐为 `min(字段大小, N)`：

```
示例 (byte_alignment = 4): int (4B) + double (8B) + char (1B)

偏移 0:  [int   ]              — 对齐到 min(4,4) = 4
偏移 4:  [   double     ]      — 对齐到 min(8,4) = 4
偏移 12: [c][pad]              — 对齐到 min(1,4) = 1，尾部填充到 4
总计: 16 字节
```

---

## ⚠️ 使用限制

| 限制 | 详情 |
|---|---|
| **非线程安全** | 不要在多线程中共享同一个 `StructsBuilder` 实例，除非自行保证外部同步 |
| **仅支持 POD 类型** | 字段必须是可平凡复制的类型，不支持具有非平凡构造函数/析构函数的类型 |
| **`operator<<` 不支持数组类型** | 请使用 `push_back(ptr, size)` 重载版本代替——对数组使用 `operator<<` 会导致编译错误 |
| **手动内存管理** | `read_data` 返回的 `void*` 需由调用者手动 `free()` |

---

## 📁 项目结构

```
StructsBuilder/
├── StructsBuilder.h   # 整个库（仅头文件）
├── LICENSE            # Apache License 2.0
├── README.md          # 英文文档
├── README.zh-CN.md    # 简体中文文档（本文件）
├── README.zh-TW.md    # 繁體中文文档
└── README.ja.md       # 日本語文档
```

---

## 📄 许可证

本项目基于 [Apache License 2.0](LICENSE) 开源。

---

*由 Sway 于 2021 年 11 月 创建。*
