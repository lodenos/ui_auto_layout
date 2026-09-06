#include "ui.h"

#include <stdio.h>

static void row(const char *name, uint32_t id, const ui_container_t *c)
{
    printf("%-10s  %3u  %7.0f %7.0f %7.0f %7.0f\n", name, id,
           c->layout.frame.x, c->layout.frame.y,
           c->layout.frame.w, c->layout.frame.h);
}

int main(void)
{
    ui_container_t store[16];
    ui_arena_t ar;
    uint32_t win, title, body, side, content;

    ui_arena_init(&ar, store, 16);

    win = ui_push(&ar);
    title = ui_push(&ar);
    body = ui_push(&ar);
    side = ui_push(&ar);
    content = ui_push(&ar);

    /* window: column, 800x600 */
    ui_at(&ar, win)->layout.display = UI_DISPLAY_BOX_FLEX;
    ui_at(&ar, win)->layout.direction = UI_DIRECTION_COLUMN;
    ui_at(&ar, win)->layout.preferred_w = 800;
    ui_at(&ar, win)->layout.preferred_h = 600;

    /* title: FIX height */
    ui_at(&ar, title)->layout.display = UI_DISPLAY_BOX_FIX;
    ui_at(&ar, title)->layout.flex = UI_FLEX_FIX;
    ui_at(&ar, title)->layout.preferred_w = 800;
    ui_at(&ar, title)->layout.preferred_h = 40;

    /* body: FILL remaining height, row */
    ui_at(&ar, body)->layout.display = UI_DISPLAY_BOX_FLEX;
    ui_at(&ar, body)->layout.direction = UI_DIRECTION_ROW;
    ui_at(&ar, body)->layout.flex = UI_FLEX_FILL;
    ui_at(&ar, body)->layout.preferred_w = 0;
    ui_at(&ar, body)->layout.preferred_h = 0;

    /* sidebar: FIX width */
    ui_at(&ar, side)->layout.display = UI_DISPLAY_BOX_FIX;
    ui_at(&ar, side)->layout.flex = UI_FLEX_FIX;
    ui_at(&ar, side)->layout.preferred_w = 200;
    ui_at(&ar, side)->layout.preferred_h = 0;

    /* content: FILL leftover width */
    ui_at(&ar, content)->layout.display = UI_DISPLAY_BOX_FIX;
    ui_at(&ar, content)->layout.flex = UI_FLEX_FILL;
    ui_at(&ar, content)->layout.preferred_w = 0;
    ui_at(&ar, content)->layout.preferred_h = 0;

    ui_add(&ar, win, title);
    ui_add(&ar, win, body);
    ui_add(&ar, body, side);
    ui_add(&ar, body, content);

    ui_layout(&ar, win);

    puts("window layout (title FIX h, body FILL, sidebar FIX w, content FILL)");
    puts("name          id        x       y       w       h");
    row("window", win, ui_at(&ar, win));
    row("title", title, ui_at(&ar, title));
    row("body", body, ui_at(&ar, body));
    row("sidebar", side, ui_at(&ar, side));
    row("content", content, ui_at(&ar, content));
    return 0;
}
