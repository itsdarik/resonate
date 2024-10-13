#include "animation.h"

#include <stdio.h>

static double ease_in_out_quadratic(double progress) {
  if (progress < 0.5) {
    return 2 * progress * progress;
  } else {
    return -1 + 2 * progress * (2 - progress);
  }
}

static int interpolate(int start, int end, double progress) {
  return start + (end - start) * ease_in_out_quadratic(progress);
}

animation_status animation_thx_deep_note(hue_stream_message_data *frame,
                                         const struct timespec *start_time) {
  struct timespec current_time = {0};
  if (clock_gettime(CLOCK_MONOTONIC, &current_time)) {
    fprintf(stderr, "clock_gettime() failed\n");
    return ANIMATION_STATUS_ERROR;
  }

  double elapsed_time = current_time.tv_sec - start_time->tv_sec +
                        (current_time.tv_nsec - start_time->tv_nsec) / 1e9;

  if (elapsed_time < 3.3) {
    // Black.
  } else if (elapsed_time < 6.3) {
    double progress = (elapsed_time - 3.3) / 3.0;
    uint16_t x = 0x38af;
    uint16_t y = 0x3134;
    uint16_t brightness = interpolate(0x0000, 0x8000, progress);
    for (int i = 0; i < 10; i++) {
      frame[i].color_value[0] = x;
      frame[i].color_value[1] = y;
      frame[i].color_value[2] = brightness;
    }
  } else if (elapsed_time < 17.0) {
    // Hold blue.
  } else if (elapsed_time < 19.0) {
    // Fade down.
    double progress = (elapsed_time - 17.0) / 2.0;
    uint16_t brightness = interpolate(0x8000, 0x4000, progress);
    for (int i = 0; i < 10; i++) {
      frame[i].color_value[2] = brightness;
    }
  }
  else if (elapsed_time < 22.0) {
    // Fade to bright white.
    double progress = (elapsed_time - 19.0) / 3.0;
    uint16_t x = interpolate(0x38af, 0x50b0, progress);
    uint16_t y = interpolate(0x3134, 0x55b6, progress);
    uint16_t brightness = interpolate(0x4000, 0xffff, progress);
    for (int i = 0; i < 10; i++) {
      frame[i].color_value[0] = x;
      frame[i].color_value[1] = y;
      frame[i].color_value[2] = brightness;
    }
  } else if (elapsed_time < 28.0) {
    // Hold bright white.
  } else if (elapsed_time < 30.5) {
    // Fade to black.
    double progress = (elapsed_time - 28.0) / 2.5;
    uint16_t brightness = interpolate(0xffff, 0x0000, progress);
    for (int i = 0; i < 10; i++) {
      frame[i].color_value[2] = brightness;
    }
  }
  else {
    return ANIMATION_STATUS_END;
  }

  return ANIMATION_STATUS_RUNNING;
}

animation_status
animation_spider_man_into_the_spider_verse(hue_stream_message_data *frame,
                                           const struct timespec *start_time) {
  return animation_thx_deep_note(frame, start_time);
}
