#include "animation.h"

#include <stdio.h>

animation_status animation_thx_deep_note(const struct timespec *start_time) {
  struct timespec current_time = {0};
  if (clock_gettime(CLOCK_MONOTONIC, &current_time)) {
    fprintf(stderr, "clock_gettime() failed\n");
    return ANIMATION_STATUS_ERROR;
  }

  double elapsed_time = current_time.tv_sec - start_time->tv_sec +
                        (current_time.tv_nsec - start_time->tv_nsec) / 1e9;

  if (elapsed_time < 5.0) {
    printf("Phase 1\n");
  } else if (elapsed_time < 10.0) {
    printf("Phase 2\n");
  } else if (elapsed_time < 15.0) {
    printf("Phase 3\n");
  } else {
    return ANIMATION_STATUS_END;
  }

  return ANIMATION_STATUS_RUNNING;
}

animation_status
animation_spider_man_into_the_spider_verse(const struct timespec *start_time) {
  return animation_thx_deep_note(start_time);
}
