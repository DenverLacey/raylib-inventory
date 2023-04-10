#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "raylib.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define INV_SIZE 25

typedef struct {
    const char *name;
} Object;

const Object OBJECTS[] = {
    {
        .name = "Food"
    },
    {
        .name = "Sword"
    },
    {
        .name = "Shield"
    }
};

typedef enum {
    ItemKind_Food,
    ItemKind_Sword,
    ItemKind_Shield,
    ItemKind_Count,
} ItemKind;

typedef struct {
    ItemKind kind;
    int max_slot_size;
    const Object *prefab;
} Item;

const Item ITEMS[ItemKind_Count] = {
    [ItemKind_Food] = {
        .kind = ItemKind_Food,
        .max_slot_size = 64,
        .prefab = &OBJECTS[0]
    },
    [ItemKind_Sword] = {
        .kind = ItemKind_Sword,
        .max_slot_size = 1,
        .prefab = &OBJECTS[1]
    },
    [ItemKind_Shield] = {
        .kind = ItemKind_Shield,
        .max_slot_size = 1,
        .prefab = &OBJECTS[2]
    }
};

typedef struct {
    int size;
    const Item *item;
} Slot;

typedef struct {
    bool occupied[INV_SIZE];
    Slot slots[INV_SIZE];
} Inventory;

bool inventory_store_item_at(Inventory *inv, const Item *item, int index) {
    Slot *slot = &inv->slots[index];
    if (inv->occupied[index]) {
        if (slot->item != item) return false;
        if (slot->size >= item->max_slot_size) return false;
        ++slot->size;
    } else {
        slot->item = item;
        slot->size = 1;
        inv->occupied[index] = true;
    }

    return true;
}

bool inventory_store_item(Inventory *inv, const Item *item) {
    int target_slot = -1;
    bool slot_found = false;
    for (int i = 0; i < INV_SIZE; ++i) {
        if (!inv->occupied[i]) {
            if (target_slot == -1) target_slot = i;
            slot_found = true;
            continue;
        }

        if (inv->slots[i].item == item &&
            inv->slots[i].size < item->max_slot_size)
        {
            ++inv->slots[i].size;
            return true;
        }
    }

    if (!slot_found) {
        return false;
    }

    Slot *slot = &inv->slots[target_slot];
    slot->item = item;
    slot->size = 1;
    inv->occupied[target_slot] = true;

    return true;
}

Slot *inventory_get_slot_at(Inventory *inv, int index) {
    if (!inv->occupied[index]) {
        return NULL;
    }

    Slot *slot = &inv->slots[index];
    return slot;
}

const Item *inventory_get_item_at(Inventory *inv, int index) {
    Slot *slot = inventory_get_slot_at(inv, index);
    if (!slot) return NULL;

    return slot->item;
}

void inventory_print(Inventory *inv) {
    printf("{\n");
    for (int i = 0; i < INV_SIZE; ++i) {
        if (!inv->occupied[i]) continue;
        Slot *s = inventory_get_slot_at(inv, i);
        printf("\t%d: %s (%d),\n", i, s->item->prefab->name, s->size);
    }
    printf("}\n");
}

typedef struct {
    Vector2 position;
    Inventory *inventory;
} Player;

int main(void) {
    #ifdef DEBUG
        SetTraceLogLevel(LOG_ALL);
    #else
        SetTraceLogLevel(LOG_ERROR);
    #endif

    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Inventory Test");

    Inventory inventory = {0};
    inventory_store_item_at(&inventory, &ITEMS[ItemKind_Sword], 0);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Shield]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Sword]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);
    inventory_store_item(&inventory, &ITEMS[ItemKind_Food]);

    inventory_print(&inventory);

    while (!WindowShouldClose()) {
        // Update =============================================================
        // Draw ===============================================================
        ClearBackground(BLACK);
        BeginDrawing();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
