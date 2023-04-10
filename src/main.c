#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

#include "raylib.h"
#include "raymath.h"

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define INV_SIZE 25

#define PLAYER_SIZE 15
#define PLAYER_SPEED 500

#define INV_UI_SIZE_FACTOR 0.5f
#define INV_UI_OPACITY 0.5f
#define INV_UI_HEADER_POS_X 15
#define INV_UI_HEADER_POS_Y 15 
#define INV_UI_HEADER_FONT_SIZE 25

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

    Player player;
    player.position = (Vector2){ WINDOW_WIDTH * INV_UI_SIZE_FACTOR, WINDOW_HEIGHT * INV_UI_SIZE_FACTOR };
    player.inventory = &inventory;

    bool inventory_open = false;
    RenderTexture2D inventory_texture = LoadRenderTexture(WINDOW_WIDTH * INV_UI_SIZE_FACTOR, WINDOW_HEIGHT * INV_UI_SIZE_FACTOR);
    char inventory_texture_slot_size_buffer[3];

    while (!WindowShouldClose()) {
        // Input ==============================================================
        float dt = GetFrameTime();

        Vector2 velocity = {0};
        if (IsKeyDown(KEY_W)) velocity.y -= 1;
        if (IsKeyDown(KEY_A)) velocity.x -= 1;
        if (IsKeyDown(KEY_S)) velocity.y += 1;
        if (IsKeyDown(KEY_D)) velocity.x += 1;
        velocity = Vector2Normalize(velocity);
        velocity = Vector2Scale(velocity, PLAYER_SPEED * dt);

        if (IsKeyPressed(KEY_I)) inventory_open = !inventory_open;
        
        // Update =============================================================
        player.position = Vector2Add(player.position, velocity);
        player.position = Vector2Clamp(
            player.position,
            (Vector2){ 0, 0 },
            (Vector2){ WINDOW_WIDTH - PLAYER_SIZE, WINDOW_HEIGHT - PLAYER_SIZE }
        );

        // Draw ===============================================================
        ClearBackground(WHITE);

        BeginDrawing();
            DrawRectangle(player.position.x, player.position.y, PLAYER_SIZE, PLAYER_SIZE, BLACK);

            if (inventory_open) {
                BeginTextureMode(inventory_texture);
                    DrawRectangle(
                        0,
                        0,
                        inventory_texture.texture.width,
                        inventory_texture.texture.height,
                        ColorAlpha(BLACK, INV_UI_OPACITY)
                    );
                    DrawText("Inventory:", INV_UI_HEADER_POS_X, INV_UI_HEADER_POS_Y, INV_UI_HEADER_FONT_SIZE, WHITE);

                    for (int i = 0; i < INV_SIZE; ++i) {
                        int pad_top = INV_UI_HEADER_POS_Y + 30;
                        int pad_left = INV_UI_HEADER_POS_X;
                        int pad_slot = 10;
                        int shift_left = i % 5;
                        int shift_down = i / 5;

                        int slot_size = 50;

                        int x = pad_left + shift_left * (slot_size + pad_slot);
                        int y = pad_top + shift_down * (slot_size + pad_slot);

                        DrawRectangle(x, y, slot_size, slot_size, YELLOW);
                        if (player.inventory->occupied[i]) {
                            DrawText(player.inventory->slots[i].item->prefab->name, x + 5, y + 5, 15, BLACK);

                            snprintf(
                                inventory_texture_slot_size_buffer,
                                sizeof(inventory_texture_slot_size_buffer),
                                "%d",
                                player.inventory->slots[i].size
                            );
                            DrawText(inventory_texture_slot_size_buffer, x + 5, y + 20, 15, BLACK);
                        }
                    }
                EndTextureMode();

                Rectangle inventory_rect = (Rectangle){
                    .width = inventory_texture.texture.width,
                    .height = -inventory_texture.texture.height,
                    .x = 0,
                    .y = 0.f
                };

                Vector2 inventory_pos = (Vector2){
                    WINDOW_WIDTH * INV_UI_SIZE_FACTOR * 0.5f,
                    WINDOW_HEIGHT * INV_UI_SIZE_FACTOR * 0.5f
                };

                DrawTextureRec(inventory_texture.texture, inventory_rect, inventory_pos, WHITE);
            }
        EndDrawing();
    }

    UnloadRenderTexture(inventory_texture);
    CloseWindow();
    return 0;
}
