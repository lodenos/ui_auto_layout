#ifndef UI_HOST_H
#define UI_HOST_H

#include <stdbool.h>
#include <stdint.h>

typedef struct ui_host_s ui_host_t;
typedef struct ui_input_s ui_input_t;

struct ui_input_s {
  int32_t mouse_x;
  int32_t mouse_y;
  bool mouse_down_left;
  bool mouse_pressed_left;
  bool mouse_released_left;
  float scroll_x;
  float scroll_y;
};

struct ui_host_s {
  void *backend;

  int32_t framebuffer_width;
  int32_t framebuffer_height;
  int32_t window_width;
  int32_t window_height;
  float content_scale_x;
  float content_scale_y;

  void (*fn_poll)(ui_host_t *host, ui_input_t *input);
  bool (*fn_should_close)(ui_host_t *host);
  void (*fn_swap)(ui_host_t *host);
  void (*fn_delete)(ui_host_t *host);
};

#endif
