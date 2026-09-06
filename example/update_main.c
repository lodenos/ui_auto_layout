/*
 * Tiny driver: only ui_container_update. No GLFW, no GL.
 * Platform is a no-op vtable (draw path exercised without a GPU).
 */
#include "ui.h"

#include <stdio.h>

static void noop_begin(ui_platform_t *platform, int32_t width, int32_t height)
{
    (void)platform;
    (void)width;
    (void)height;
}

static void noop_draw_container(ui_platform_t *platform, ui_container_t *container)
{
    (void)platform;
    printf("draw (%.0f, %.0f, %.0f, %.0f)\n",
           container->layout.frame.x, container->layout.frame.y,
           container->layout.frame.w, container->layout.frame.h);
}

static void noop_end(ui_platform_t *platform)
{
    (void)platform;
}

int main(void)
{
    ui_container_t store[4];
    ui_arena_t arena;
    ui_platform_t platform;
    uint32_t root;
    uint32_t child;

    ui_arena_init(&arena, store, 4);
    root = ui_push(&arena);
    child = ui_push(&arena);
    ui_at(&arena, root)->layout.preferred_w = 200;
    ui_at(&arena, root)->layout.preferred_h = 100;
    ui_at(&arena, root)->layout.direction = UI_DIRECTION_ROW;
    ui_at(&arena, child)->layout.flex = UI_FLEX_FIX;
    ui_at(&arena, child)->layout.preferred_w = 50;
    ui_at(&arena, child)->layout.preferred_h = 20;
    ui_add(&arena, root, child);

    platform = (ui_platform_t){
        .fn_begin = noop_begin,
        .fn_draw_container = noop_draw_container,
        .fn_end = noop_end,
    };

    ui_container_update(&arena, root, NULL, &platform);
    return 0;
}
