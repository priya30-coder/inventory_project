/*
 * InventoryManager.cpp
 * C++ layer: menu, input validation, STL (vector + sort).
 */

#include "InventoryManager.hpp"
#include "inventory.h"

#include <algorithm>    /* std::sort */
#include <cctype>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>       /* std::vector */

/* =========================================================== formatting helpers */

static const int COL_ID   = 6;
static const int COL_NAME = 42;
static const int COL_QTY  = 10;
static const int COL_PRC  = 12;

void InventoryManager::printDivider()
{
    std::cout << std::string(COL_ID + COL_NAME + COL_QTY + COL_PRC + 7, '-') << "\n";
}

void InventoryManager::printHeader()
{
    printDivider();
    std::cout << std::left
              << std::setw(COL_ID)   << " ID"
              << std::setw(COL_NAME) << " Name"
              << std::setw(COL_QTY)  << " Qty"
              << std::setw(COL_PRC)  << " Price"
              << "\n";
    printDivider();
}

void InventoryManager::printItem(const Item &it)
{
    std::cout << std::left
              << " " << std::setw(COL_ID - 1)   << it.id
              << " " << std::setw(COL_NAME - 1)  << it.name
              << " " << std::setw(COL_QTY - 1)   << it.quantity
              << " " << std::fixed << std::setprecision(2)
                     << std::setw(COL_PRC - 1)   << it.price
              << "\n";
}

/* =========================================================== validated input */

int InventoryManager::readPositiveInt(const std::string &prompt)
{
    for (;;) {
        std::cout << prompt;
        int v;
        if (std::cin >> v && v > 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [!] Must be a positive integer. Try again.\n";
    }
}

int InventoryManager::readNonNegInt(const std::string &prompt)
{
    for (;;) {
        std::cout << prompt;
        int v;
        if (std::cin >> v && v >= 0) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [!] Must be 0 or greater. Try again.\n";
    }
}

float InventoryManager::readNonNegFloat(const std::string &prompt)
{
    for (;;) {
        std::cout << prompt;
        float v;
        if (std::cin >> v && v >= 0.0f) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return v;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "  [!] Must be 0.00 or greater. Try again.\n";
    }
}

std::string InventoryManager::readNonEmptyStr(const std::string &prompt,
                                               std::size_t        max_len)
{
    for (;;) {
        std::cout << prompt;
        std::string s;
        std::getline(std::cin, s);

        /* trim leading / trailing whitespace */
        std::size_t start = s.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            std::cout << "  [!] Name cannot be empty. Try again.\n";
            continue;
        }
        std::size_t end = s.find_last_not_of(" \t\r\n");
        s = s.substr(start, end - start + 1);

        if (s.empty()) {
            std::cout << "  [!] Name cannot be empty. Try again.\n";
            continue;
        }
        if (s.size() > max_len) {
            std::cout << "  [!] Name too long (max " << max_len << " chars). Try again.\n";
            continue;
        }
        return s;
    }
}

/* =========================================================== CRUD wrappers */

bool InventoryManager::addItem(int id, const std::string &name,
                                int qty, float price)
{
    Item it{};
    it.id         = id;
    it.quantity   = qty;
    it.price      = price;
    it.is_deleted = 0;

    std::strncpy(it.name, name.c_str(), NAME_LEN - 1);
    it.name[NAME_LEN - 1] = '\0';

    return add_item(&it) == 1;
}

bool InventoryManager::viewItem(int id)
{
    Item it{};
    if (!get_item(id, &it))
        return false;

    printHeader();
    printItem(it);
    printDivider();
    return true;
}

bool InventoryManager::updateItem(int id)
{
    /* Verify it exists first */
    Item existing{};
    if (!get_item(id, &existing)) {
        std::cout << "  [!] Item #" << id << " not found or already deleted.\n";
        return false;
    }

    std::cout << "  Current record:\n";
    printHeader();
    printItem(existing);
    printDivider();
    std::cout << "  Enter new values (leave blank to keep current):\n";

    /* --- name --- */
    std::cout << "  New name [" << existing.name << "]: ";
    std::string line;
    std::getline(std::cin, line);
    std::string new_name = line.empty() ? existing.name : line;
    if (new_name.empty()) new_name = existing.name;

    /* --- quantity --- */
    int new_qty = readNonNegInt("  New quantity [" +
                                std::to_string(existing.quantity) + "]: ");

    /* --- price --- */
    float new_price = readNonNegFloat("  New price [" +
                                      std::to_string(existing.price) + "]: ");

    Item updated{};
    updated.id         = id;
    updated.quantity   = new_qty;
    updated.price      = new_price;
    updated.is_deleted = 0;
    std::strncpy(updated.name, new_name.c_str(), NAME_LEN - 1);
    updated.name[NAME_LEN - 1] = '\0';

    return update_item(id, &updated) == 1;
}

bool InventoryManager::deleteItem(int id)
{
    return delete_item(id) == 1;
}

void InventoryManager::listItems(bool sort_by_name)
{
    /* ---- use std::vector ---- */
    std::vector<Item> items(MAX_ITEMS);
    int count = list_items(items.data(), MAX_ITEMS);
    items.resize(static_cast<std::size_t>(count));

    if (items.empty()) {
        std::cout << "  (no active items)\n";
        return;
    }

    /* ---- use std::sort ---- */
    if (sort_by_name) {
        std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
            return std::strcmp(a.name, b.name) < 0;
        });
    } else {
        std::sort(items.begin(), items.end(), [](const Item &a, const Item &b) {
            return a.id < b.id;
        });
    }

    printHeader();
    for (const auto &it : items)
        printItem(it);
    printDivider();
    std::cout << "  Total: " << items.size() << " item(s)\n";
}

/* =========================================================== interactive menu */

void InventoryManager::runMenu()
{
    const std::string BANNER =
        "\n"
        "  ╔══════════════════════════════════════╗\n"
        "  ║      HYBRID INVENTORY MANAGER        ║\n"
        "  ╚══════════════════════════════════════╝\n";

    for (;;) {
        std::cout << BANNER
                  << "  1) Add item\n"
                  << "  2) View item by ID\n"
                  << "  3) Update item\n"
                  << "  4) Delete item\n"
                  << "  5) List all items\n"
                  << "  6) Exit\n"
                  << "  Choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "  [!] Invalid choice.\n";
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        std::cout << "\n";

        switch (choice) {

        case 1: {   /* ---- Add item ---- */
            int         id    = readPositiveInt ("  ID       : ");
            std::string name  = readNonEmptyStr ("  Name     : ", NAME_LEN - 1);
            int         qty   = readNonNegInt   ("  Quantity : ");
            float       price = readNonNegFloat ("  Price    : ");

            if (addItem(id, name, qty, price))
                std::cout << "  [OK] Item #" << id << " added.\n";
            else
                std::cout << "  [!] Failed – duplicate ID or write error.\n";
            break;
        }

        case 2: {   /* ---- View item ---- */
            int id = readPositiveInt("  ID: ");
            if (!viewItem(id))
                std::cout << "  [!] Item #" << id << " not found.\n";
            break;
        }

        case 3: {   /* ---- Update item ---- */
            int id = readPositiveInt("  ID to update: ");
            if (updateItem(id))
                std::cout << "  [OK] Item #" << id << " updated.\n";
            else
                std::cout << "  [!] Update failed.\n";
            break;
        }

        case 4: {   /* ---- Delete item ---- */
            int id = readPositiveInt("  ID to delete: ");
            if (deleteItem(id))
                std::cout << "  [OK] Item #" << id << " deleted.\n";
            else
                std::cout << "  [!] Item not found or already deleted.\n";
            break;
        }

        case 5: {   /* ---- List all ---- */
            std::cout << "  Sort by – (1) ID  (2) Name : ";
            int s = 1;
            if (std::cin >> s)
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            else {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            }
            std::cout << "\n";
            listItems(s == 2);
            break;
        }

        case 6:
            std::cout << "  Goodbye.\n";
            return;

        default:
            std::cout << "  [!] Choose 1–6.\n";
            break;
        }

        std::cout << "\n";
    }
}
