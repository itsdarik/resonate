#pragma once

#include <time.h>

typedef enum animation animation;
enum animation {
  ANIMATION_THX_DEEP_NOTE,
  ANIMATION_SPIDER_MAN_INTO_THE_SPIDER_VERSE
};

typedef enum animation_status animation_status;
enum animation_status {
  ANIMATION_STATUS_ERROR = -1,
  ANIMATION_STATUS_END = 0,
  ANIMATION_STATUS_RUNNING = 1
};

/**
 * @brief Animate the lights to the THX Deep Note.
 *
 * @param start_time The time when the animation started.
 *
 * @return The status of the animation.
 */
animation_status animation_thx_deep_note(const struct timespec *start_time);

/**
 * @brief Animate the lights to the opening logos of Spider-Man: Into the
 * Spider-Verse.
 *
 * @param start_time The time when the animation started.
 *
 * @return The status of the animation.
 */
animation_status
animation_spider_man_into_the_spider_verse(const struct timespec *start_time);
