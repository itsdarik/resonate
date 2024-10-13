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

  if (elapsed_time < 5.0) {
    double progress = elapsed_time / 5.0;
    uint16_t x = 0x0000;
    uint16_t y = interpolate(0x0000, 0xffff, progress);
    uint16_t brightness = interpolate(0x0000, 0xffff, progress);
    for (int i = 0; i < 10; i++) {
      frame[i].color_value[0] = x;
      frame[i].color_value[1] = y;
      frame[i].color_value[2] = brightness;
    }
    printf("brightness: %d\n", brightness);
  } else if (elapsed_time < 10.0) {
    // Do nothing.
  } else if (elapsed_time < 15.0) {
    // Do nothing.
  } else {
    return ANIMATION_STATUS_END;
  }

  return ANIMATION_STATUS_RUNNING;
}

animation_status
animation_spider_man_into_the_spider_verse(hue_stream_message_data *frame,
                                           const struct timespec *start_time) {
  return animation_thx_deep_note(frame, start_time);
}
