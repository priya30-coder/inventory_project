/*
 * inventory.c
 * C backend: binary-file storage using fread / fwrite / fseek.
 * Every Item occupies sizeof(Item) bytes at offset  id_index * sizeof(Item)
 * (sequential, fixed-length records).
 */

#include "inventory.h"

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ helpers */

/* Open the data file.  mode follows fopen() conventions. */
static FILE *open_db(const char *mode)
{
    return fopen(DATA_FILE, mode);
}

/*
 * Scan the file for a record whose id matches `id`.
 * Returns the byte offset of that record on success, -1L on failure.
 * The file must already be open and positioned at the start.
 */
static long find_offset(FILE *fp, int id)
{
    Item   tmp;
    long   offset = 0L;

    rewind(fp);
    while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (tmp.id == id)
            return offset;
        offset += (long)sizeof(Item);
    }
    return -1L;
}

/* ------------------------------------------------------------------ public API */

/*
 * add_item
 * Appends a new record.  Rejects duplicate IDs (including soft-deleted ones
 * so that the slot can never be re-used accidentally).
 */
int add_item(const Item *item)
{
    FILE *fp;
    Item  tmp;

    if (!item || item->id <= 0)
        return 0;

    /* Check for duplicate */
    fp = open_db("rb");
    if (fp) {
        while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
            if (tmp.id == item->id) {
                fclose(fp);
                return 0;   /* duplicate */
            }
        }
        fclose(fp);
    }

    /* Append */
    fp = open_db("ab");
    if (!fp)
        return 0;

    if (fwrite(item, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

/*
 * get_item
 * Finds the first active record with the given id and copies it into *out.
 */
int get_item(int id, Item *out)
{
    FILE *fp;
    Item  tmp;

    if (!out || id <= 0)
        return 0;

    fp = open_db("rb");
    if (!fp)
        return 0;

    while (fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (tmp.id == id && !tmp.is_deleted) {
            *out = tmp;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

/*
 * update_item
 * Overwrites the matching (non-deleted) record in place.
 */
int update_item(int id, const Item *updated)
{
    FILE *fp;
    long  offset;

    if (!updated || id <= 0)
        return 0;

    fp = open_db("r+b");
    if (!fp)
        return 0;

    offset = find_offset(fp, id);
    if (offset < 0) {
        fclose(fp);
        return 0;
    }

    /* Verify it is not already deleted */
    {
        Item current;
        fseek(fp, offset, SEEK_SET);
        if (fread(&current, sizeof(Item), 1, fp) != 1 || current.is_deleted) {
            fclose(fp);
            return 0;
        }
    }

    /* Write updated record at same offset; preserve the original id */
    {
        Item to_write = *updated;
        to_write.id  = id;              /* guard against caller changing id */
        to_write.is_deleted = 0;

        fseek(fp, offset, SEEK_SET);
        if (fwrite(&to_write, sizeof(Item), 1, fp) != 1) {
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 1;
}

/*
 * delete_item
 * Soft-delete: sets is_deleted = 1 in the file record.
 */
int delete_item(int id)
{
    FILE *fp;
    long  offset;
    Item  current;

    if (id <= 0)
        return 0;

    fp = open_db("r+b");
    if (!fp)
        return 0;

    offset = find_offset(fp, id);
    if (offset < 0) {
        fclose(fp);
        return 0;
    }

    fseek(fp, offset, SEEK_SET);
    if (fread(&current, sizeof(Item), 1, fp) != 1 || current.is_deleted) {
        fclose(fp);
        return 0;
    }

    current.is_deleted = 1;
    fseek(fp, offset, SEEK_SET);
    if (fwrite(&current, sizeof(Item), 1, fp) != 1) {
        fclose(fp);
        return 0;
    }

    fclose(fp);
    return 1;
}

/*
 * list_items
 * Copies up to max_items active records into buffer.
 * Returns the number of records copied.
 */
int list_items(Item *buffer, int max_items)
{
    FILE *fp;
    Item  tmp;
    int   count = 0;

    if (!buffer || max_items <= 0)
        return 0;

    fp = open_db("rb");
    if (!fp)
        return 0;

    while (count < max_items && fread(&tmp, sizeof(Item), 1, fp) == 1) {
        if (!tmp.is_deleted)
            buffer[count++] = tmp;
    }

    fclose(fp);
    return count;
}
