# Hybrid Inventory Manager

A console-based inventory manager that demonstrates C/C++ interoperability:

- **C layer** (`inventory.c`) – binary file I/O with `fread` / `fwrite` / `fseek`
- **C++ layer** (`InventoryManager.cpp`) – OOP wrapper, `std::vector`, `std::sort`, validated menu

Data persists across restarts in **`inventory.dat`** (fixed-length binary records).

---

## Project Structure

```
inventory_project/
├── include/
│   ├── inventory.h          # C struct + extern-C API declarations
│   └── InventoryManager.hpp # C++ class declaration
├── src/
│   ├── inventory.c          # C backend (file storage)
│   ├── InventoryManager.cpp # C++ menu + STL
│   └── main.cpp             # Entry point
├── Makefile
├── CMakeLists.txt
└── README.md
```

---

## Build & Run

### Option A – Make (recommended)

```bash
cd inventory_project
make            # compiles into ./inventory_manager
./inventory_manager
```

Clean everything (including the data file):

```bash
make clean
```

### Option B – CMake

```bash
cd inventory_project
cmake -S . -B build
cmake --build build
./build/inventory_manager
```

> **Requirements**: GCC ≥ 9 (or Clang ≥ 10), Make or CMake ≥ 3.14.

---

## Usage

```
  ╔══════════════════════════════════════╗
  ║      HYBRID INVENTORY MANAGER        ║
  ╚══════════════════════════════════════╝
  1) Add item
  2) View item by ID
  3) Update item
  4) Delete item
  5) List all items
  6) Exit
```

- All inputs are validated; the program re-prompts on bad input instead of crashing.
- **Listing** sorts by ID (default) or Name (choose at prompt).

---

## Data Format

Each record is a fixed-length C struct written verbatim to `inventory.dat`:

| Field        | Type       | Notes                      |
|--------------|------------|----------------------------|
| `id`         | `int`      | Positive, must be unique   |
| `name`       | `char[40]` | Non-empty string           |
| `quantity`   | `int`      | ≥ 0                        |
| `price`      | `float`    | ≥ 0.00                     |
| `is_deleted` | `int`      | 0 = active, 1 = soft-deleted |

Record at file position `offset = record_index × sizeof(Item)` — enables O(1) fseek updates.

---

## C Backend API

```c
int add_item    (const Item *item);            // 1 = OK, 0 = fail/duplicate
int get_item    (int id, Item *out);           // 1 = found & active
int update_item (int id, const Item *updated); // 1 = updated in-place
int delete_item (int id);                      // 1 = soft-deleted
int list_items  (Item *buffer, int max_items); // returns count copied
```

---

## Test Cases

| # | Action | Expected result |
|---|--------|-----------------|
| 1 | Add items with IDs 1, 2, 3 → exit → restart → **List all** | All three items appear (persistence verified) |
| 2 | Add item ID 1 → **Update** name/quantity/price → exit → restart → **View** ID 1 | Updated values shown, not originals |
| 3 | Add item ID 5 → **Delete** ID 5 → **View** ID 5 | "Item not found" (soft-delete hides record) |
| 4 | **List all** after delete in test 3 | Item #5 does not appear in the list |
| 5 | Add item ID 2 → try **Add** again with ID 2 | "Failed – duplicate ID" message; only one record stored |

---

## Design Notes

- **Soft delete**: `is_deleted = 1` keeps the file position valid for future reads; the slot is never reclaimed to avoid ID collisions.
- **fseek strategy**: `find_offset()` does a linear scan to locate the record, then `fseek` jumps to that byte for in-place write — making update and delete O(n) scan but O(1) write.
- **extern "C"**: the header wraps declarations in `extern "C"` so the C++ compiler uses C linkage when calling the C translation unit.
- **STL usage**: `std::vector<Item>` buffers the active records; `std::sort` with a lambda sorts by id or name before display.
