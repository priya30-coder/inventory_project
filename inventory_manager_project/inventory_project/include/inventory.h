#ifndef INVENTORY_H
#define INVENTORY_H

#ifdef __cplusplus
extern "C" {
#endif

#define NAME_LEN     40
#define DATA_FILE    "inventory.dat"

typedef struct {
    int   id;
    char  name[NAME_LEN];
    int   quantity;
    float price;
    int   is_deleted;   /* 0 = active, 1 = soft-deleted */
} Item;

/*
 * Returns 1 on success, 0 on failure.
 * add_item  – rejects duplicate IDs.
 * get_item  – fills *out; returns 0 if not found or deleted.
 * update_item – overwrites the record in-place.
 * delete_item – sets is_deleted = 1 in-place.
 * list_items  – copies active items into buffer; returns count copied.
 */
int add_item    (const Item *item);
int get_item    (int id, Item *out);
int update_item (int id, const Item *updated);
int delete_item (int id);
int list_items  (Item *buffer, int max_items);

#ifdef __cplusplus
}
#endif

#endif /* INVENTORY_H */
