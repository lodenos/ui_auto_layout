#include "ui.h"

#include <math.h>

#define UI_MAX(a, b) ((a) > (b) ? (a) : (b))
#define UI_KIDS 256

static inline int ui_row(ui_direction_e d)
{
    return d == UI_DIRECTION_ROW || d == UI_DIRECTION_ROW_REVERSE;
}

static inline int ui_rev(ui_direction_e d)
{
    return d == UI_DIRECTION_ROW_REVERSE || d == UI_DIRECTION_COLUMN_REVERSE;
}

static float ui_clamp(float v, float lo, float hi)
{
    if (v < lo)
        v = lo;
    if (hi > 0.f && v > hi)
        v = hi;
    return v;
}

static ui_direction_e ui_dir_of(const ui_container_t *n)
{
    if (n->layout.display == UI_DISPLAY_BOX_GRID)
        return UI_DIRECTION_COLUMN;
    return n->layout.direction;
}

static ui_flex_e ui_flex_of(const ui_container_t *child, ui_display_e pd)
{
    if (pd == UI_DISPLAY_BOX_FIX || pd == UI_DISPLAY_BOX_GRID)
        return UI_FLEX_FIX;
    return child->layout.flex;
}

static float ui_main(const ui_container_t *n, int row, int measured)
{
    float w = measured ? n->layout.measured_w : n->layout.frame.w;
    float h = measured ? n->layout.measured_h : n->layout.frame.h;
    return row ? w : h;
}

static float ui_cross(const ui_container_t *n, int row, int measured)
{
    float w = measured ? n->layout.measured_w : n->layout.frame.w;
    float h = measured ? n->layout.measured_h : n->layout.frame.h;
    return row ? h : w;
}

/* 9-point -> start(0) / center(1) / end(2) on the cross axis. */
static int ui_cross_slot(ui_align_e a, int row)
{
    if (row) {
        if (a == UI_ALIGN_TOP_LEFT || a == UI_ALIGN_TOP_CENTER || a == UI_ALIGN_TOP_RIGHT)
            return 0;
        if (a == UI_ALIGN_BOTTOM_LEFT || a == UI_ALIGN_BOTTOM_CENTER || a == UI_ALIGN_BOTTOM_RIGHT)
            return 2;
        return 1;
    }
    if (a == UI_ALIGN_TOP_LEFT || a == UI_ALIGN_LEFT || a == UI_ALIGN_BOTTOM_LEFT)
        return 0;
    if (a == UI_ALIGN_TOP_RIGHT || a == UI_ALIGN_RIGHT || a == UI_ALIGN_BOTTOM_RIGHT)
        return 2;
    return 1;
}

static float ui_cross_off(ui_align_e a, int row, float line, float child)
{
    int s = ui_cross_slot(a, row);
    if (s == 1)
        return (line - child) * 0.5f;
    if (s == 2)
        return line - child;
    return 0.f;
}

static void ui_space_split(ui_space sp, int n, float extra, float *lead, float *mid)
{
    *lead = 0.f;
    *mid = 0.f;
    if (n <= 0)
        return;
    if (extra < 0.f)
        extra = 0.f;
    switch (sp) {
    case UI_SPACE_FIX:
        break;
    case UI_SPACE_BETWEEN:
        if (n > 1)
            *mid = extra / (float)(n - 1);
        break;
    case UI_SPACE_AROUND:
        *mid = extra / (float)n;
        *lead = *mid * 0.5f;
        break;
    case UI_SPACE_EVENLY:
        *mid = extra / (float)(n + 1);
        *lead = *mid;
        break;
    }
}

void ui_arena_init(ui_arena_t *a, ui_container_t *buf, uint32_t cap)
{
    a->buf = buf;
    a->cap = cap;
    a->used = 0;
}

uint32_t ui_push(ui_arena_t *a)
{
    uint32_t id;
    ui_container_t *c;

    if (a->used >= a->cap)
        return UI_NONE;
    id = a->used++;
    c = &a->buf[id];
    *c = (ui_container_t){0};
    c->interface.parent = UI_NONE;
    c->interface.first_child = UI_NONE;
    c->interface.next_sib = UI_NONE;
    c->layout.display = UI_DISPLAY_BOX_FLEX;
    c->layout.direction = UI_DIRECTION_ROW;
    c->layout.align = UI_ALIGN_TOP_LEFT;
    c->layout.space = UI_SPACE_FIX;
    c->layout.wrap = UI_WRAP_NONE;
    c->layout.flex = UI_FLEX_FIX;
    c->scroll.mode = UI_SCROLL_NONE;
    c->scroll.bar = UI_SCROLLBAR_HIDE;
    return id;
}

void ui_add(ui_arena_t *a, uint32_t parent, uint32_t child)
{
    ui_container_t *p = &a->buf[parent];
    ui_container_t *c = &a->buf[child];
    uint32_t it;

    c->interface.parent = parent;
    c->interface.next_sib = UI_NONE;
    if (p->interface.first_child == UI_NONE) {
        p->interface.first_child = child;
    } else {
        it = p->interface.first_child;
        while (a->buf[it].interface.next_sib != UI_NONE)
            it = a->buf[it].interface.next_sib;
        a->buf[it].interface.next_sib = child;
    }
    p->interface.child_count++;
}

static void ui_measure(ui_arena_t *a, uint32_t id);

static void ui_measure_leaf(ui_container_t *n)
{
    float w = n->layout.preferred_w;
    float h = n->layout.preferred_h;
    if (n->layout.flex == UI_FLEX_FIT) {
        w += n->layout.pad_left + n->layout.pad_right;
        h += n->layout.pad_top + n->layout.pad_bottom;
    }
    n->layout.measured_w = ui_clamp(w, n->layout.min_w, n->layout.max_w);
    n->layout.measured_h = ui_clamp(h, n->layout.min_h, n->layout.max_h);
}

static void ui_measure(ui_arena_t *a, uint32_t id)
{
    ui_container_t *n = &a->buf[id];
    uint32_t ch;
    int row, k = 0;
    ui_direction_e dir;
    float pad_x, pad_y, content_main = 0.f, content_cross = 0.f;
    float mw, mh;

    if (n->interface.first_child == UI_NONE) {
        ui_measure_leaf(n);
        return;
    }

    for (ch = n->interface.first_child; ch != UI_NONE; ch = a->buf[ch].interface.next_sib)
        ui_measure(a, ch);

    dir = ui_dir_of(n);
    row = ui_row(dir);
    pad_x = n->layout.pad_left + n->layout.pad_right;
    pad_y = n->layout.pad_top + n->layout.pad_bottom;

    for (ch = n->interface.first_child; ch != UI_NONE; ch = a->buf[ch].interface.next_sib) {
        ui_container_t *c = &a->buf[ch];
        float cm = ui_main(c, row, 1);
        float cc = ui_cross(c, row, 1);
        if (k)
            content_main += n->layout.gap;
        content_main += cm;
        content_cross = UI_MAX(content_cross, cc);
        k++;
    }

    if (row) {
        mw = content_main + pad_x;
        mh = content_cross + pad_y;
    } else {
        mw = content_cross + pad_x;
        mh = content_main + pad_y;
    }

    if (n->layout.flex != UI_FLEX_FIT && n->layout.display != UI_DISPLAY_BOX_FLEX) {
        if (n->layout.preferred_w > 0.f)
            mw = n->layout.preferred_w;
        if (n->layout.preferred_h > 0.f)
            mh = n->layout.preferred_h;
    } else if (n->layout.flex == UI_FLEX_FIX || n->layout.flex == UI_FLEX_FILL) {
        if (n->layout.preferred_w > 0.f)
            mw = n->layout.preferred_w;
        if (n->layout.preferred_h > 0.f)
            mh = n->layout.preferred_h;
    }

    n->layout.measured_w = ui_clamp(mw, n->layout.min_w, n->layout.max_w);
    n->layout.measured_h = ui_clamp(mh, n->layout.min_h, n->layout.max_h);
    n->scroll.content_w = row ? content_main : content_cross;
    n->scroll.content_h = row ? content_cross : content_main;
}

static void ui_place(ui_arena_t *a, uint32_t id, float x, float y, float w, float h);

static void ui_place(ui_arena_t *a, uint32_t id, float x, float y, float w, float h)
{
    ui_container_t *n = &a->buf[id];
    uint32_t kids[UI_KIDS];
    int nk = 0, nfill = 0, i, row, reverse;
    ui_direction_e dir;
    float inner_x, inner_y, inner_w, inner_h;
    float inner_main, inner_cross, origin_main, origin_cross;
    float definite = 0.f, extra, lead, mid, cursor, fill_each;
    float gap;

    n->layout.frame.x = x;
    n->layout.frame.y = y;
    n->layout.frame.w = w;
    n->layout.frame.h = h;

    if (n->interface.first_child == UI_NONE)
        return;

    dir = ui_dir_of(n);
    row = ui_row(dir);
    reverse = ui_rev(dir);
    gap = n->layout.gap;

    inner_x = x + n->layout.pad_left - n->scroll.offset_x;
    inner_y = y + n->layout.pad_top - n->scroll.offset_y;
    inner_w = w - n->layout.pad_left - n->layout.pad_right;
    inner_h = h - n->layout.pad_top - n->layout.pad_bottom;
    if (inner_w < 0.f)
        inner_w = 0.f;
    if (inner_h < 0.f)
        inner_h = 0.f;

    origin_main = row ? inner_x : inner_y;
    origin_cross = row ? inner_y : inner_x;
    inner_main = row ? inner_w : inner_h;
    inner_cross = row ? inner_h : inner_w;

    {
        uint32_t ch = n->interface.first_child;
        while (ch != UI_NONE && nk < UI_KIDS) {
            kids[nk++] = ch;
            ch = a->buf[ch].interface.next_sib;
        }
    }

    for (i = 0; i < nk; i++) {
        ui_container_t *c = &a->buf[kids[i]];
        ui_flex_e fx = ui_flex_of(c, n->layout.display);
        if (fx == UI_FLEX_FILL)
            nfill++;
        else
            definite += ui_main(c, row, 1);
    }
    if (nk > 1)
        definite += gap * (float)(nk - 1);

    extra = inner_main - definite;
    if (extra < 0.f)
        extra = 0.f;
    fill_each = (nfill > 0) ? extra / (float)nfill : 0.f;

    if (nfill > 0)
        lead = mid = 0.f;
    else
        ui_space_split(n->layout.space, nk, extra, &lead, &mid);

    if (reverse)
        cursor = origin_main + inner_main - lead;
    else
        cursor = origin_main + lead;

    for (i = 0; i < nk; i++) {
        ui_container_t *c = &a->buf[kids[i]];
        ui_flex_e fx = ui_flex_of(c, n->layout.display);
        float main_len, cross_len, cw, chh, off;
        float used;

        if (fx == UI_FLEX_FILL)
            main_len = fill_each;
        else
            main_len = ui_main(c, row, 1);

        cross_len = ui_cross(c, row, 1);
        /* stretch cross when the child did not pick a preferred size */
        if (row) {
            if (c->layout.preferred_h <= 0.f && inner_cross > 0.f)
                cross_len = inner_cross;
            cw = main_len;
            chh = cross_len;
        } else {
            if (c->layout.preferred_w <= 0.f && inner_cross > 0.f)
                cross_len = inner_cross;
            cw = cross_len;
            chh = main_len;
        }
        cw = ui_clamp(cw, c->layout.min_w, c->layout.max_w);
        chh = ui_clamp(chh, c->layout.min_h, c->layout.max_h);
        used = row ? cw : chh;
        off = ui_cross_off(n->layout.align, row, inner_cross, row ? chh : cw);

        if (reverse)
            cursor -= used;

        if (row)
            ui_place(a, kids[i], cursor, origin_cross + off, cw, chh);
        else
            ui_place(a, kids[i], origin_cross + off, cursor, cw, chh);

        if (reverse)
            cursor -= gap + mid;
        else
            cursor += used + gap + mid;
    }

    {
        uint32_t ch;
        float max_r = 0.f, max_b = 0.f;
        for (ch = n->interface.first_child; ch != UI_NONE; ch = a->buf[ch].interface.next_sib) {
            ui_container_t *c = &a->buf[ch];
            float r = (c->layout.frame.x - inner_x) + c->layout.frame.w;
            float b = (c->layout.frame.y - inner_y) + c->layout.frame.h;
            if (r > max_r)
                max_r = r;
            if (b > max_b)
                max_b = b;
        }
        n->scroll.content_w = max_r;
        n->scroll.content_h = max_b;
    }
}

static void ui_snap_tree(ui_arena_t *a, uint32_t id)
{
    ui_container_t *n = &a->buf[id];
    uint32_t ch;
    n->layout.frame.x = roundf(n->layout.frame.x);
    n->layout.frame.y = roundf(n->layout.frame.y);
    n->layout.frame.w = roundf(n->layout.frame.w);
    n->layout.frame.h = roundf(n->layout.frame.h);
    for (ch = n->interface.first_child; ch != UI_NONE; ch = a->buf[ch].interface.next_sib)
        ui_snap_tree(a, ch);
}

void ui_layout(ui_arena_t *a, uint32_t root)
{
    ui_container_t *n = &a->buf[root];
    float w, h;

    ui_measure(a, root);
    w = n->layout.preferred_w;
    h = n->layout.preferred_h;
    if (w <= 0.f)
        w = n->layout.measured_w;
    if (h <= 0.f)
        h = n->layout.measured_h;
    ui_place(a, root, n->layout.frame.x, n->layout.frame.y, w, h);
    ui_snap_tree(a, root);
}


void ui_container_draw(ui_arena_t *arena, uint32_t node, ui_platform_t *platform)
{
    ui_container_t *container;
    uint32_t child;

    if (!arena || !platform || node == UI_NONE)
        return;
    if (!platform->fn_draw_container)
        return;

    container = ui_at(arena, node);
    platform->fn_draw_container(platform, container);

    child = container->interface.first_child;
    while (child != UI_NONE) {
        ui_container_draw(arena, child, platform);
        child = ui_at(arena, child)->interface.next_sib;
    }
}

void ui_container_update(ui_arena_t *arena, uint32_t root,
                         ui_host_t *host, ui_platform_t *platform)
{
    ui_input_t input;
    int32_t width;
    int32_t height;

    if (!arena || root == UI_NONE)
        return;

    input = (ui_input_t){0};
    if (host && host->fn_poll)
        host->fn_poll(host, &input);

    ui_layout(arena, root);

    width = host ? host->framebuffer_width : 0;
    height = host ? host->framebuffer_height : 0;
    if (width <= 0)
        width = (int32_t)ui_at(arena, root)->layout.frame.w;
    if (height <= 0)
        height = (int32_t)ui_at(arena, root)->layout.frame.h;

    if (platform) {
        if (platform->fn_begin)
            platform->fn_begin(platform, width, height);
        ui_container_draw(arena, root, platform);
        if (platform->fn_end)
            platform->fn_end(platform);
    }

    if (host && host->fn_swap)
        host->fn_swap(host);

    (void)input; /* hit-test / dispatch lands in a later slice */
}
