#pragma once

#include "inventory.h"

#include <string>
#include <vector>

/*
 * InventoryManager
 * Thin C++ wrapper around the C backend.
 * Owns all console I/O and calls the five C functions.
 */
class InventoryManager {
public:
    /* ---------- CRUD wrappers (return true on success) ---------- */
    bool addItem    (int id, const std::string &name, int qty, float price);
    bool viewItem   (int id);          /* prints to stdout */
    bool updateItem (int id);          /* interactive re-entry */
    bool deleteItem (int id);

    /* Lists all active items sorted by id (or name if sort_by_name==true) */
    void listItems  (bool sort_by_name = false);

    /* ---------- interactive menu ---------- */
    void runMenu();

private:
    /* ---------- helpers ---------- */
    static void printItem   (const Item &it);
    static void printHeader ();
    static void printDivider();

    /* validated input readers */
    static int         readPositiveInt  (const std::string &prompt);
    static int         readNonNegInt    (const std::string &prompt);
    static float       readNonNegFloat  (const std::string &prompt);
    static std::string readNonEmptyStr  (const std::string &prompt, std::size_t max_len);

    static constexpr int MAX_ITEMS = 4096;
};
