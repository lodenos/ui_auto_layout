#ifndef UI_H
#define UI_H

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

typedef struct ui_container_s ui_container_t;

struct ui_container_s {
  struct {} interface;
  struct {} layout;
  struct {} scroll;
  struct {} style;
};

#endif
