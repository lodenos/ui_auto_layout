# ui_auto_layout

C11 CPU-only flex layout. Nodes live in an arena; the tree is indices
(`parent`, `first_child`, `next_sib`), not pointers. After `ui_layout()`,
read `layout.frame.{x,y,w,h}` (pixel-snapped).

## Build

```bash
make test          # test_layout + test_fixture_v1
make fixture       # QA v1 acceptance only
make update_main && ./update_main   # ui_container_update, no GLFW/GL
make demo          # print a sample window table
```

Cold-run for QA:

```bash
cc -std=c11 -O2 -Wall -Wextra -Iinc -o test_fixture_v1 test/test_fixture_v1.c src/ui.c -lm
./test_fixture_v1   # expect: fixture v1 GREEN
```

`ui_platform_t` is a draw vtable; `ui_container_update` drives layout + draw with an optional `ui_host_t` (NULL = no window).


Equivalent:

```bash
cc -std=c11 -O2 -Wall -Wextra -Iinc -o test_layout test/test_layout.c src/ui.c -lm
./test_layout
```

## Layout pass

1. **Measure FIT** bottom-up: a `UI_FLEX_FIT` node is padding + children + `gap`.
2. **Allocate FILL** top-down: leftover main-axis space is split evenly among `UI_FLEX_FILL` siblings.
3. **Place** with `ui_space` (when there is no FILL sibling) and 9-point `ui_align_e` on the cross axis.
4. **Snap** each frame with `roundf`.

Wrap is unimplemented: only `UI_WRAP_NONE` is honoured. `UI_DISPLAY_BOX_GRID` is a
stub and is treated as a column of `FIX` children.

## Reading rects

```c
ui_container_t store[64];
ui_arena_t ar;
uint32_t root, child;

ui_arena_init(&ar, store, 64);
root = ui_push(&ar);
child = ui_push(&ar);
ui_at(&ar, root)->layout.preferred_w = 800;
ui_at(&ar, root)->layout.preferred_h = 600;
ui_add(&ar, root, child);
ui_layout(&ar, root);

/* integer pixels after snap */
float x = ui_at(&ar, child)->layout.frame.x;
float y = ui_at(&ar, child)->layout.frame.y;
float w = ui_at(&ar, child)->layout.frame.w;
float h = ui_at(&ar, child)->layout.frame.h;
```

`UI_NONE` (`0xFFFFFFFF`) means no parent / no sibling / no child.

Default node: `UI_DISPLAY_BOX_FLEX`, `UI_DIRECTION_ROW`, `UI_FLEX_FIX`,
`UI_ALIGN_TOP_LEFT`, `UI_SPACE_FIX`.

Root origin is whatever `layout.frame.x/y` was before `ui_layout` (usually 0,0).
A FIT root with `preferred_* == 0` sizes to content.
