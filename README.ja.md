# StructsBuilder

🌐 [English](README.md) | [简体中文](README.zh-CN.md) | [繁體中文](README.zh-TW.md) | **日本語**

軽量なヘッダーオンリーの C++ ライブラリで、**実行時に構造体のメモリレイアウトを動的に構築**できます。任意の型のフィールドをビルダーにプッシュし、実際の C/C++ 構造体と同一のレイアウト（正しいバイトアライメントを含む）を持つ連続メモリブロック（`void*`）として読み出せます。

---

## 📦 特徴

- **ヘッダーオンリー** — `#include "StructsBuilder.h"` だけで利用可能、リンク不要
- **型非依存** — `int`、`double`、カスタム POD 型など、任意のフィールドをプッシュ可能
- **柔軟な挿入** — 末尾追加、先頭挿入、任意位置への挿入をサポート
- **ストリーム演算子** — チェーン可能な `<<` 構文で素早く組み立て
- **バイトアライメント制御** — デフォルト（自然）アライメント、またはカスタムパッキングを指定可能
- **バッチプッシュ** — 配列ポインタ＋サイズを渡して複数要素を一括プッシュ
- **RAII 自動解放** — デストラクタで全メモリを自動的に解放

---

## 🚀 クイックスタート

```cpp
#include "StructsBuilder.h"
#include <cstdio>

// 目的の構造体レイアウト
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

    // フィールドごとに構造体を構築
    builder.push_back(id);
    builder.push_back(value);
    builder.push_back(flag);

    // 構築済みメモリを読み出し
    void* data = nullptr;
    std::size_t size = builder.read_data(data);

    // キャストして使用
    MyStruct* s = reinterpret_cast<MyStruct*>(data);
    printf("id=%d, value=%.2f, flag=%c\n", s->id, s->value, s->flag);
    // 出力: id=42, value=3.14, flag=A

    free(data);
    return 0;
}
```

---

## 📖 API リファレンス

### コンストラクタとデストラクタ

| メソッド | 説明 |
|---|---|
| `StructsBuilder()` | デフォルトコンストラクタ — 空のビルダーを作成 |
| `~StructsBuilder()` | デストラクタ — 内部で確保した全メモリを解放 |

### フィールドの追加

| メソッド | 説明 |
|---|---|
| `push_back(const T& data)` | 末尾にフィールドを1つ追加 |
| `push_back(const T* data, size_t size)` | 末尾にフィールドの配列を追加 |
| `push_front(const T& data)` | 先頭にフィールドを1つ挿入 |
| `push_front(const T* data, size_t size)` | 先頭にフィールドの配列を挿入 |
| `insert(size_t pos, const T& data)` | 指定位置 `pos` にフィールドを1つ挿入（0始まり） |
| `insert(size_t pos, const T* data, size_t size)` | 指定位置 `pos` からフィールドの配列を挿入 |
| `operator<<(const T& data)` | ストリーム演算子、`push_back` と同等 |

> すべての挿入メソッドは `StructsBuilder&` を返し、メソッドチェーンが可能です：
> ```cpp
> builder.push_back(x).push_back(y).push_back(z);
> // または
> builder << x << y << z;
> ```

### 読み出しとメモリ

| メソッド | 説明 |
|---|---|
| `read_data(void*& data, size_t alignment = 0)` | 構造体を連続メモリブロックとして構築。合計バイトサイズを返す。`0`（デフォルト）で自然アライメント、またはカスタム値（例: `1`、`4`、`8`）でパッキングを指定。 |
| `empty()` | フィールドが未追加なら `true` を返す |
| `clear()` | 全フィールドを削除し、内部メモリを解放 |

> **重要:** `read_data` が返す `void*` は `malloc` で確保されています。使用後は呼び出し側で `free()` を呼ぶ必要があります。

---

## 🔧 バイトアライメント

`read_data` は2つのアライメントモードをサポートします：

### デフォルトアライメント（`byte_alignment = 0`）

コンパイラの自然な構造体パッキングルールを模倣：
- 各フィールドは自身のサイズに等しい境界にアライメント
- 構造体の合計サイズは最大フィールドサイズの倍数にパディング

```
例: int (4B) + double (8B) + char (1B)

オフセット 0:  [int   ][pad   ]      — 4バイトデータ + 4バイトパディング
オフセット 8:  [   double     ]      — 8バイト、自然アライメント
オフセット 16: [c][  padding  ]      — 1バイト + 7バイト末尾パディング
合計: 24バイト
```

### カスタムアライメント（`byte_alignment = N`）

各フィールドの有効アライメントは `min(フィールドサイズ, N)`：

```
例 (byte_alignment = 4): int (4B) + double (8B) + char (1B)

オフセット 0:  [int   ]              — min(4,4) = 4 にアライメント
オフセット 4:  [   double     ]      — min(8,4) = 4 にアライメント
オフセット 12: [c][pad]              — min(1,4) = 1 にアライメント、末尾を4にパディング
合計: 16バイト
```

---

## ⚠️ 制限事項

| 制限 | 詳細 |
|---|---|
| **スレッドセーフではない** | 外部同期なしに複数スレッドで同一の `StructsBuilder` インスタンスを共有しないでください |
| **POD 型のみ** | フィールドはトリビアルコピー可能な型である必要があります。非トリビアルなコンストラクタ/デストラクタを持つ型はサポートされません |
| **`operator<<` で配列型は不可** | 代わりに `push_back(ptr, size)` オーバーロードを使用してください — 配列に `operator<<` を使うとコンパイルエラーになります |
| **手動メモリ管理** | `read_data` が返す `void*` は呼び出し側で `free()` する必要があります |

---

## 📁 プロジェクト構成

```
StructsBuilder/
├── StructsBuilder.h   # ライブラリ全体（ヘッダーオンリー）
├── LICENSE            # Apache License 2.0
├── README.md          # 英語ドキュメント
├── README.zh-CN.md    # 简体中文ドキュメント
├── README.zh-TW.md    # 繁體中文ドキュメント
└── README.ja.md       # 日本語ドキュメント（本ファイル）
```