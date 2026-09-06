#include "ui.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static int g_fail;

static int near(float a, float b)
{
    return fabsf(a - b) <= 0.01f;
}

static void expect(const char *name, const ui_container_t *c,
                   float x, float y, float w, float h)
{
    if (!near(c->layout.frame.x, x) || !near(c->layout.frame.y, y) ||
        !near(c->layout.frame.w, w) || !near(c->layout.frame.h, h)) {
        fprintf(stderr,
                "FAIL %s: got (%.2f %.2f %.2f %.2f) want (%.2f %.2f %.2f %.2f)\n",
                name, c->layout.frame.x, c->layout.frame.y, c->layout.frame.w,
                c->layout.frame.h, x, y, w, h);
        g_fail = 1;
    } else {
        printf("ok  %s (%.0f, %.0f, %.0f, %.0f)\n", name, x, y, w, h);
    }
}

static uint32_t box(ui_arena_t *a, float w, float h, ui_flex_e flex)
{
    uint32_t id = ui_push(a);
    ui_container_t *c = ui_at(a, id);
    c->layout.display = UI_DISPLAY_BOX_FIX;
    c->layout.flex = flex;
    c->layout.preferred_w = w;
    c->layout.preferred_h = h;
    return id;
}

int main(void)
{
    /* --- empty tree --- */
    {
        ui_container_t store[4];
        ui_arena_t ar;
        uint32_t r;
        ui_arena_init(&ar, store, 4);
        r = ui_push(&ar);
        ui_at(&ar, r)->layout.preferred_w = 100;
        ui_at(&ar, r)->layout.preferred_h = 50;
        ui_layout(&ar, r);
        expect("empty.root", ui_at(&ar, r), 0, 0, 100, 50);
        assert(near(ui_at(&ar, r)->layout.frame.w, 100.f));
        assert(ui_at(&ar, r)->interface.first_child == UI_NONE);
        assert(ui_at(&ar, r)->interface.child_count == 0);
    }

    /* --- row of FIX --- */
    {
        ui_container_t store[8];
        ui_arena_t ar;
        uint32_t r, a, b, c;
        ui_arena_init(&ar, store, 8);
        r = ui_push(&ar);
        a = box(&ar, 50, 20, UI_FLEX_FIX);
        b = box(&ar, 50, 20, UI_FLEX_FIX);
        c = box(&ar, 50, 20, UI_FLEX_FIX);
        ui_at(&ar, r)->layout.preferred_w = 300;
        ui_at(&ar, r)->layout.preferred_h = 100;
        ui_at(&ar, r)->layout.direction = UI_DIRECTION_ROW;
        ui_add(&ar, r, a);
        ui_add(&ar, r, b);
        ui_add(&ar, r, c);
        ui_layout(&ar, r);
        expect("row.root", ui_at(&ar, r), 0, 0, 300, 100);
        expect("row.a", ui_at(&ar, a), 0, 0, 50, 20);
        expect("row.b", ui_at(&ar, b), 50, 0, 50, 20);
        expect("row.c", ui_at(&ar, c), 100, 0, 50, 20);
        assert(near(ui_at(&ar, c)->layout.frame.x, 100.f));
    }

    /* --- FIT parent + gap --- */
    {
        ui_container_t store[8];
        ui_arena_t ar;
        uint32_t r, a, b;
        ui_arena_init(&ar, store, 8);
        r = ui_push(&ar);
        a = box(&ar, 30, 10, UI_FLEX_FIX);
        b = box(&ar, 40, 10, UI_FLEX_FIX);
        ui_at(&ar, r)->layout.flex = UI_FLEX_FIT;
        ui_at(&ar, r)->layout.display = UI_DISPLAY_BOX_FLEX;
        ui_at(&ar, r)->layout.gap = 5;
        ui_add(&ar, r, a);
        ui_add(&ar, r, b);
        ui_layout(&ar, r);
        expect("fit.root", ui_at(&ar, r), 0, 0, 75, 10);
        expect("fit.a", ui_at(&ar, a), 0, 0, 30, 10);
        expect("fit.b", ui_at(&ar, b), 35, 0, 40, 10);
        assert(near(ui_at(&ar, r)->layout.frame.w, 75.f));
    }

    /* --- FILL splits leftover --- */
    {
        ui_container_t store[8];
        ui_arena_t ar;
        uint32_t r, a, b;
        ui_arena_init(&ar, store, 8);
        r = ui_push(&ar);
        a = box(&ar, 0, 20, UI_FLEX_FILL);
        b = box(&ar, 0, 20, UI_FLEX_FILL);
        ui_at(&ar, r)->layout.preferred_w = 200;
        ui_at(&ar, r)->layout.preferred_h = 50;
        ui_add(&ar, r, a);
        ui_add(&ar, r, b);
        ui_layout(&ar, r);
        expect("fill.a", ui_at(&ar, a), 0, 0, 100, 20);
        expect("fill.b", ui_at(&ar, b), 100, 0, 100, 20);
        assert(near(ui_at(&ar, a)->layout.frame.w, 100.f));
        assert(near(ui_at(&ar, b)->layout.frame.w, 100.f));
    }

    /* --- column + center --- */
    {
        ui_container_t store[8];
        ui_arena_t ar;
        uint32_t r, a, b;
        ui_arena_init(&ar, store, 8);
        r = ui_push(&ar);
        a = box(&ar, 40, 20, UI_FLEX_FIX);
        b = box(&ar, 80, 20, UI_FLEX_FIX);
        ui_at(&ar, r)->layout.preferred_w = 200;
        ui_at(&ar, r)->layout.preferred_h = 100;
        ui_at(&ar, r)->layout.direction = UI_DIRECTION_COLUMN;
        ui_at(&ar, r)->layout.align = UI_ALIGN_CENTER;
        ui_add(&ar, r, a);
        ui_add(&ar, r, b);
        ui_layout(&ar, r);
        expect("col.root", ui_at(&ar, r), 0, 0, 200, 100);
        expect("col.a", ui_at(&ar, a), 80, 0, 40, 20);
        expect("col.b", ui_at(&ar, b), 60, 20, 80, 20);
        assert(near(ui_at(&ar, a)->layout.frame.x, 80.f));
        assert(near(ui_at(&ar, b)->layout.frame.x, 60.f));
    }

    if (g_fail) {
        fprintf(stderr, "some asserts failed\n");
        return 1;
    }
    puts("all layout checks passed");
    return 0;
}
