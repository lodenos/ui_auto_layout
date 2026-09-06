/*
 * QA v1 acceptance: arena ownership, deterministic flex FIX/FIT/FILL
 * (row + column), ui_container_update with no GLFW/GL, frames within 1px.
 */
#include "ui.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static int g_fail;

static int within_one_pixel(float got, float want)
{
    return fabsf(got - want) <= 1.0f;
}

static void expect_frame(uint32_t node_id, const char *label,
                         const ui_container_t *container,
                         float expect_x, float expect_y,
                         float expect_w, float expect_h)
{
    if (!within_one_pixel(container->layout.frame.x, expect_x) ||
        !within_one_pixel(container->layout.frame.y, expect_y) ||
        !within_one_pixel(container->layout.frame.w, expect_w) ||
        !within_one_pixel(container->layout.frame.h, expect_h)) {
        fprintf(stderr,
                "FAIL node %u (%s): got (%.2f %.2f %.2f %.2f) "
                "want (%.2f %.2f %.2f %.2f)\n",
                node_id, label,
                container->layout.frame.x, container->layout.frame.y,
                container->layout.frame.w, container->layout.frame.h,
                expect_x, expect_y, expect_w, expect_h);
        g_fail = 1;
    } else {
        printf("ok  node %u %s\n", node_id, label);
    }
}

static uint32_t make_box(ui_arena_t *arena, float width, float height,
                         ui_flex_e flex)
{
    uint32_t node_id;
    ui_container_t *container;

    node_id = ui_push(arena);
    container = ui_at(arena, node_id);
    container->layout.display = UI_DISPLAY_BOX_FLEX;
    container->layout.flex = flex;
    container->layout.preferred_w = width;
    container->layout.preferred_h = height;
    return node_id;
}

typedef struct {
    uint32_t count;
    uint32_t ids[32];
    float frames[32][4];
} record_backend_t;

static void record_begin(ui_platform_t *platform, int32_t width, int32_t height)
{
    record_backend_t *record;

    (void)width;
    (void)height;
    record = (record_backend_t *)platform->backend;
    record->count = 0;
}

static void record_draw_container(ui_platform_t *platform,
                                  ui_container_t *container)
{
    record_backend_t *record;
    uint32_t index;

    record = (record_backend_t *)platform->backend;
    if (record->count >= 32)
        return;
    index = record->count++;
    /* node id = offset in arena buf; tests pass arena via style.color hack? */
    record->ids[index] = container->style.color; /* we stash id here */
    record->frames[index][0] = container->layout.frame.x;
    record->frames[index][1] = container->layout.frame.y;
    record->frames[index][2] = container->layout.frame.w;
    record->frames[index][3] = container->layout.frame.h;
}

static void record_end(ui_platform_t *platform)
{
    (void)platform;
}

static void run_row_fixture(void)
{
    ui_container_t store[8];
    ui_arena_t arena;
    uint32_t root;
    uint32_t fix_child;
    uint32_t fit_child;
    uint32_t fill_a;
    uint32_t fill_b;
    int pass;

    for (pass = 0; pass < 2; ++pass) {
        ui_arena_init(&arena, store, 8);
        root = ui_push(&arena);
        ui_at(&arena, root)->layout.display = UI_DISPLAY_BOX_FLEX;
        ui_at(&arena, root)->layout.direction = UI_DIRECTION_ROW;
        ui_at(&arena, root)->layout.preferred_w = 300;
        ui_at(&arena, root)->layout.preferred_h = 40;
        ui_at(&arena, root)->layout.flex = UI_FLEX_FIX;

        fix_child = make_box(&arena, 50, 40, UI_FLEX_FIX);
        fit_child = make_box(&arena, 30, 40, UI_FLEX_FIT);
        fill_a = make_box(&arena, 0, 40, UI_FLEX_FILL);
        fill_b = make_box(&arena, 0, 40, UI_FLEX_FILL);

        ui_add(&arena, root, fix_child);
        ui_add(&arena, root, fit_child);
        ui_add(&arena, root, fill_a);
        ui_add(&arena, root, fill_b);

        ui_layout(&arena, root);

        expect_frame(root, "row.root", ui_at(&arena, root), 0, 0, 300, 40);
        expect_frame(fix_child, "row.fix", ui_at(&arena, fix_child), 0, 0, 50, 40);
        expect_frame(fit_child, "row.fit", ui_at(&arena, fit_child), 50, 0, 30, 40);
        /* leftover 220 split across two FILL */
        expect_frame(fill_a, "row.fill_a", ui_at(&arena, fill_a), 80, 0, 110, 40);
        expect_frame(fill_b, "row.fill_b", ui_at(&arena, fill_b), 190, 0, 110, 40);
    }
}

static void run_column_fixture(void)
{
    ui_container_t store[8];
    ui_arena_t arena;
    uint32_t root;
    uint32_t fix_child;
    uint32_t fit_child;
    uint32_t fill_child;
    int pass;

    for (pass = 0; pass < 2; ++pass) {
        ui_arena_init(&arena, store, 8);
        root = ui_push(&arena);
        ui_at(&arena, root)->layout.display = UI_DISPLAY_BOX_FLEX;
        ui_at(&arena, root)->layout.direction = UI_DIRECTION_COLUMN;
        ui_at(&arena, root)->layout.preferred_w = 100;
        ui_at(&arena, root)->layout.preferred_h = 200;
        ui_at(&arena, root)->layout.flex = UI_FLEX_FIX;

        fix_child = make_box(&arena, 100, 40, UI_FLEX_FIX);
        fit_child = make_box(&arena, 100, 20, UI_FLEX_FIT);
        fill_child = make_box(&arena, 100, 0, UI_FLEX_FILL);

        ui_add(&arena, root, fix_child);
        ui_add(&arena, root, fit_child);
        ui_add(&arena, root, fill_child);

        ui_layout(&arena, root);

        expect_frame(root, "col.root", ui_at(&arena, root), 0, 0, 100, 200);
        expect_frame(fix_child, "col.fix", ui_at(&arena, fix_child), 0, 0, 100, 40);
        expect_frame(fit_child, "col.fit", ui_at(&arena, fit_child), 0, 40, 100, 20);
        expect_frame(fill_child, "col.fill", ui_at(&arena, fill_child), 0, 60, 100, 140);
    }
}

static void run_update_no_gl(void)
{
    ui_container_t store[4];
    ui_arena_t arena;
    uint32_t root;
    uint32_t child;
    record_backend_t record;
    ui_platform_t platform;

    ui_arena_init(&arena, store, 4);
    root = ui_push(&arena);
    child = make_box(&arena, 50, 20, UI_FLEX_FIX);
    ui_at(&arena, root)->layout.preferred_w = 200;
    ui_at(&arena, root)->layout.preferred_h = 100;
    ui_at(&arena, root)->layout.direction = UI_DIRECTION_ROW;
    ui_at(&arena, root)->style.color = root;
    ui_at(&arena, child)->style.color = child;
    ui_add(&arena, root, child);

    memset(&record, 0, sizeof record);
    platform = (ui_platform_t){
        .backend = &record,
        .fn_begin = record_begin,
        .fn_draw_container = record_draw_container,
        .fn_draw_text = NULL,
        .fn_end = record_end,
        .fn_delete = NULL,
    };

    /* host NULL — no GLFW/GL */
    ui_container_update(&arena, root, NULL, &platform);

    if (record.count != 2) {
        fprintf(stderr, "FAIL update draw count: got %u want 2\n", record.count);
        g_fail = 1;
    } else {
        printf("ok  update drew %u containers via ui_platform_t (no host)\n",
               record.count);
    }
    expect_frame(child, "update.child", ui_at(&arena, child), 0, 0, 50, 20);
}

int main(void)
{
    run_row_fixture();
    run_column_fixture();
    run_update_no_gl();

    if (g_fail) {
        fprintf(stderr, "fixture v1 RED\n");
        return 1;
    }
    puts("fixture v1 GREEN");
    return 0;
}
