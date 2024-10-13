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
    printf("Phase 1: %f\n", progress);
    frame[0].color_value[0] = interpolate(0, 255, progress);
  } else if (elapsed_time < 10.0) {
    double progress = (elapsed_time - 5.0) / 5.0;
    printf("Phase 2: %f\n", progress);
  } else if (elapsed_time < 15.0) {
    double progress = (elapsed_time - 10.0) / 5.0;
    printf("Phase 3: %f\n", progress);
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
