#ifndef UI_PLATFORM_H
#define UI_PLATFORM_H

#include <stdint.h>

typedef struct ui_container_s ui_container_t;
typedef struct ui_platform_s ui_platform_t;

struct ui_platform_s {
  void *backend;

  void (*fn_begin)(ui_platform_t *platform, int32_t width, int32_t height);
  void (*fn_draw_container)(ui_platform_t *platform, ui_container_t *container);
  void (*fn_draw_text)(ui_platform_t *platform, void *text);
  void (*fn_end)(ui_platform_t *platform);
  void (*fn_delete)(ui_platform_t *platform);
};

#endif
