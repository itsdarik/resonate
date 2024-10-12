#pragma once

typedef enum animation animation;
enum animation {
  ANIMATION_THX_DEEP_NOTE,
  ANIMATION_SPIDER_MAN_INTO_THE_SPIDER_VERSE
};

/**
 * @brief Animate the lights to the THX Deep Note.
 */
void animation_thx_deep_note();

/**
 * @brief Animate the lights to the opening logos of Spider-Man: Into the
 * Spider-Verse.
 */
void animation_spider_man_into_the_spider_verse();
