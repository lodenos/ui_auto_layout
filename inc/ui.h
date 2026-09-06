#ifndef UI_H
#define UI_H

/*
 * CPU-only C11 flex box layout. Nodes live in an arena; the tree is
 * parent / first_child / next_sib indices (not pointers).
 *
 * Wrap is a stub: only UI_WRAP_NONE is honoured.
 * Grid is a stub: UI_DISPLAY_BOX_GRID is laid out as a column.
 */

#include <stdint.h>

#include "ui_host.h"
#include "ui_platform.h"

#define UI_NONE ((uint32_t)0xFFFFFFFFu)

typedef enum {
    UI_ALIGN_TOP_LEFT,
    UI_ALIGN_TOP_CENTER,
    UI_ALIGN_TOP_RIGHT,
    UI_ALIGN_LEFT,
    UI_ALIGN_CENTER,
    UI_ALIGN_RIGHT,
    UI_ALIGN_BOTTOM_LEFT,
    UI_ALIGN_BOTTOM_CENTER,
    UI_ALIGN_BOTTOM_RIGHT,
} ui_align_e;

typedef enum {
    UI_DIRECTION_ROW,
    UI_DIRECTION_ROW_REVERSE,
    UI_DIRECTION_COLUMN,
    UI_DIRECTION_COLUMN_REVERSE,
} ui_direction_e;

typedef enum {
    UI_DISPLAY_BOX_FIX,
    UI_DISPLAY_BOX_FLEX,
    UI_DISPLAY_BOX_GRID,
} ui_display_e;

typedef enum {
    UI_FLEX_FIX,
    UI_FLEX_FILL,
    UI_FLEX_FIT,
} ui_flex_e;

typedef enum {
    UI_SCROLL_NONE,
    UI_SCROLL_ACTIVE,
} ui_scroll;

typedef enum {
    UI_SCROLLBAR_HIDE,
    UI_SCROLLBAR_SHOW,
} ui_scrollbar;

typedef enum {
    UI_SPACE_FIX,
    UI_SPACE_AROUND,
    UI_SPACE_BETWEEN,
    UI_SPACE_EVENLY,
} ui_space;

typedef enum {
    UI_WRAP_NONE,
    UI_WRAP,
    UI_WRAP_REVERSE,
} ui_wrap_e;

typedef struct ui_container_s {
    struct {
        uint32_t parent;
        uint32_t first_child;
        uint32_t next_sib;
        uint32_t child_count;
    } interface;

    struct {
        ui_display_e display;
        ui_direction_e direction;
        ui_align_e align;
        ui_space space;
        ui_wrap_e wrap;
        ui_flex_e flex;
        float gap;
        float pad_top, pad_right, pad_bottom, pad_left;
        float preferred_w, preferred_h;
        float min_w, min_h;
        float max_w, max_h; /* 0 = unbounded */
        float measured_w, measured_h;
        struct {
            float x, y, w, h;
        } frame;
    } layout;

    struct {
        ui_scroll mode;
        ui_scrollbar bar;
        float offset_x, offset_y;
        float content_w, content_h;
    } scroll;

    struct {
        uint32_t color;
    } style;
} ui_container_t;

typedef struct ui_arena_s {
    ui_container_t *buf;
    uint32_t cap;
    uint32_t used;
} ui_arena_t;

void ui_arena_init(ui_arena_t *a, ui_container_t *buf, uint32_t cap);
uint32_t ui_push(ui_arena_t *a);
void ui_add(ui_arena_t *a, uint32_t parent, uint32_t child);
void ui_layout(ui_arena_t *a, uint32_t root);
void ui_container_draw(ui_arena_t *arena, uint32_t node, ui_platform_t *platform);
void ui_container_update(ui_arena_t *arena, uint32_t root,
                        ui_host_t *host, ui_platform_t *platform);

static inline ui_container_t *ui_at(ui_arena_t *a, uint32_t id)
{
    return &a->buf[id];
}

#endif
