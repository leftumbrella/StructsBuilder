# StructsBuilder

🌐 [English](README.md) | [简体中文](README.zh-CN.md) | **繁體中文** | [日本語](README.ja.md)

一個輕量級、僅標頭檔的 C++ 函式庫，用於**在執行時期動態建構結構體記憶體佈局**。將任意型別的欄位逐一推入建構器，即可讀取一塊連續記憶體（`void*`），其佈局與真實的 C/C++ 結構體完全一致——包含正確的位元組對齊。

---

## 📦 特性

- **僅標頭檔** — 只需 `#include "StructsBuilder.h"`，無需連結額外函式庫
- **型別無關** — 可推入 `int`、`double`、自訂 POD 型別等任意欄位
- **彈性插入** — 支援尾部追加、頭部前插、任意位置插入
- **串流運算子** — 可鏈式呼叫的 `<<` 語法，快速組裝結構體
- **位元組對齊控制** — 支援預設（自然）對齊，也可指定自訂對齊方式
- **批次推入** — 傳入陣列指標 + 大小，一次推入多個元素
- **RAII 自動清理** — 解構時自動釋放所有記憶體

---

## 🚀 快速開始

```cpp
#include "StructsBuilder.h"
#include <cstdio>

// 目標結構體佈局
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

    // 逐欄位建構結構體
    builder.push_back(id);
    builder.push_back(value);
    builder.push_back(flag);

    // 讀取建構好的記憶體
    void* data = nullptr;
    std::size_t size = builder.read_data(data);

    // 強制轉型並使用
    MyStruct* s = reinterpret_cast<MyStruct*>(data);
    printf("id=%d, value=%.2f, flag=%c\n", s->id, s->value, s->flag);
    // 輸出: id=42, value=3.14, flag=A

    free(data);
    return 0;
}
```

---

## 📖 API 參考

### 建構與解構

| 方法 | 說明 |
|---|---|
| `StructsBuilder()` | 預設建構函式 — 建立一個空的建構器 |
| `~StructsBuilder()` | 解構函式 — 釋放所有內部配置的記憶體 |

### 新增欄位

| 方法 | 說明 |
|---|---|
| `push_back(const T& data)` | 在末尾追加一個欄位 |
| `push_back(const T* data, size_t size)` | 在末尾追加一組欄位（陣列） |
| `push_front(const T& data)` | 在頭部前插一個欄位 |
| `push_front(const T* data, size_t size)` | 在頭部前插一組欄位（陣列） |
| `insert(size_t pos, const T& data)` | 在指定位置 `pos` 插入一個欄位（從 0 開始） |
| `insert(size_t pos, const T* data, size_t size)` | 從指定位置 `pos` 開始插入一組欄位 |
| `operator<<(const T& data)` | 串流運算子，等同於 `push_back` |

> 所有插入方法均回傳 `StructsBuilder&`，支援鏈式呼叫：
> ```cpp
> builder.push_back(x).push_back(y).push_back(z);
> // 或者
> builder << x << y << z;
> ```

### 讀取與記憶體

| 方法 | 說明 |
|---|---|
| `read_data(void*& data, size_t alignment = 0)` | 將結構體建構為一塊連續記憶體。回傳總位元組大小。傳入 `0`（預設）使用自然對齊，或傳入自訂值（如 `1`、`4`、`8`）指定對齊方式。 |
| `empty()` | 若未推入任何欄位則回傳 `true` |
| `clear()` | 移除所有欄位並釋放內部記憶體 |

> **重要提示：** `read_data` 回傳的 `void*` 由 `malloc` 配置，呼叫者需負責在使用完畢後呼叫 `free()` 釋放。

---

## 🔧 位元組對齊

`read_data` 支援兩種對齊模式：

### 預設對齊（`byte_alignment = 0`）

模擬編譯器的自然結構體對齊規則：
- 每個欄位對齊到等於其自身大小的邊界
- 結構體總大小填充為最大欄位大小的倍數

```
範例: int (4B) + double (8B) + char (1B)

偏移 0:  [int   ][pad   ]      — 4 位元組資料 + 4 位元組填充
偏移 8:  [   double     ]      — 8 位元組，自然對齊
偏移 16: [c][  padding  ]      — 1 位元組 + 7 位元組尾部填充
總計: 24 位元組
```

### 自訂對齊（`byte_alignment = N`）

每個欄位的有效對齊為 `min(欄位大小, N)`：

```
範例 (byte_alignment = 4): int (4B) + double (8B) + char (1B)

偏移 0:  [int   ]              — 對齊到 min(4,4) = 4
偏移 4:  [   double     ]      — 對齊到 min(8,4) = 4
偏移 12: [c][pad]              — 對齊到 min(1,4) = 1，尾部填充到 4
總計: 16 位元組
```

---

## ⚠️ 使用限制

| 限制 | 詳情 |
|---|---|
| **非執行緒安全** | 請勿在多執行緒中共享同一個 `StructsBuilder` 實例，除非自行保證外部同步 |
| **僅支援 POD 型別** | 欄位必須是可平凡複製的型別，不支援具有非平凡建構函式/解構函式的型別 |
| **`operator<<` 不支援陣列型別** | 請使用 `push_back(ptr, size)` 多載版本代替——對陣列使用 `operator<<` 會導致編譯錯誤 |
| **手動記憶體管理** | `read_data` 回傳的 `void*` 需由呼叫者手動 `free()` |

---

## 📁 專案結構

```
StructsBuilder/
├── StructsBuilder.h   # 整個函式庫（僅標頭檔）
├── LICENSE            # Apache License 2.0
├── README.md          # 英文文件
├── README.zh-CN.md    # 简体中文文件
├── README.zh-TW.md    # 繁體中文文件（本文件）
└── README.ja.md       # 日本語文件
```

---

## 📄 授權條款

本專案基於 [Apache License 2.0](LICENSE) 開源。

---

*由 Z.S. 於 2021 年 11 月為 GTTC 專案建立。*
