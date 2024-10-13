#include "animation.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct animation_phase animation_phase;
struct animation_phase {
  double start_time;
  void (*animate)(hue_stream_message_data *frame, int channel_count,
                  double progress);
};

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

static void set_all_same(hue_stream_message_data *frame, int channel_count,
                         int x, int y, int brightness) {
  for (int i = 0; i < channel_count; i++) {
    frame[i].color_value[0] = x;
    frame[i].color_value[1] = y;
    frame[i].color_value[2] = brightness;
  }
}

static void set_all_same_brightness(hue_stream_message_data *frame,
                                    int channel_count, int brightness) {
  for (int i = 0; i < channel_count; i++) {
    frame[i].color_value[2] = brightness;
  }
}

#define BRIGHTNESS_ZERO 0x0000
#define BRIGHTNESS_LOW 0x00f0
#define BRIGHTNESS_HALF 0x7fff
#define BRIGHTNESS_MAX 0xffff

#define COLOR_BLUE_X 0x38af
#define COLOR_BLUE_Y 0x3134

#define COLOR_WHITE_X 0x50b0
#define COLOR_WHITE_Y 0x55b6

static animation_status animate(hue_stream_message_data *frame,
                                int channel_count,
                                const struct timespec *start_time,
                                const animation_phase *phases, int num_phases) {
  struct timespec current_time = {0};
  if (clock_gettime(CLOCK_MONOTONIC, &current_time)) {
    fprintf(stderr, "clock_gettime() failed\n");
    return ANIMATION_STATUS_ERROR;
  }

  const double elapsed_time =
      current_time.tv_sec - start_time->tv_sec +
      (current_time.tv_nsec - start_time->tv_nsec) / 1e9;

  for (int i = 0; i < num_phases; i++) {
    const double phase_start_time = phases[i].start_time;
    const double phase_end_time =
        i < num_phases - 1 ? phases[i + 1].start_time : 0;
    if (elapsed_time >= phase_start_time && elapsed_time < phase_end_time) {
      const double phase_duration = phase_end_time - phase_start_time;
      const double phase_progress =
          (elapsed_time - phase_start_time) / phase_duration;
      phases[i].animate(frame, channel_count, phase_progress);
      return ANIMATION_STATUS_RUNNING;
    }
  }

  return ANIMATION_STATUS_END;
}

static void animate_hold(hue_stream_message_data *frame, int channel_count,
                         double progress) {
  (void)frame;
  (void)channel_count;
  (void)progress;
}

static void animate_fade_to_blue(hue_stream_message_data *frame,
                                 int channel_count, double progress) {
  set_all_same(frame, channel_count, COLOR_BLUE_X, COLOR_BLUE_Y,
               interpolate(BRIGHTNESS_ZERO, BRIGHTNESS_HALF, progress));
}

static void animate_fade_to_dim(hue_stream_message_data *frame,
                                int channel_count, double progress) {
  set_all_same(frame, channel_count,
               interpolate(COLOR_BLUE_X, COLOR_WHITE_X, progress),
               interpolate(COLOR_BLUE_Y, COLOR_WHITE_Y, progress),
               interpolate(BRIGHTNESS_HALF, BRIGHTNESS_LOW, progress));
}

static void animate_fade_to_white(hue_stream_message_data *frame,
                                  int channel_count, double progress) {
  set_all_same_brightness(
      frame, channel_count,
      interpolate(BRIGHTNESS_LOW, BRIGHTNESS_MAX, progress));
}

static void animate_fade_to_off(hue_stream_message_data *frame,
                                int channel_count, double progress) {
  set_all_same_brightness(
      frame, channel_count,
      interpolate(BRIGHTNESS_MAX, BRIGHTNESS_ZERO, progress));
}

animation_status animation_thx_deep_note(hue_stream_message_data *frame,
                                         int channel_count,
                                         const struct timespec *start_time) {
  const animation_phase phases[] = {
      {0.0, animate_hold},           {3.3, animate_fade_to_blue},
      {6.3, animate_hold},           {17.0, animate_fade_to_dim},
      {19.0, animate_fade_to_white}, {22.5, animate_hold},
      {28.0, animate_fade_to_off},   {30.5, animate_hold},
  };

  const int num_phases = sizeof(phases) / sizeof(phases[0]);
  return animate(frame, channel_count, start_time, phases, num_phases);
}

static bool light_is_on(hue_stream_message_data *frame, int channel_count,
                        int light) {
  if (light < 0 || light >= channel_count) {
    fprintf(stderr, "light_is_on() called with invalid light index %d\n",
            light);
    return false;
  }
  return frame[light].color_value[2] > 0;
}

static bool light_to_random_color(hue_stream_message_data *frame,
                                  int channel_count, int light) {
  if (light < 0 || light >= channel_count) {
    fprintf(stderr,
            "light_to_random_color() called with invalid light index %d\n",
            light);
    return false;
  }
  frame[light].color_value[0] = rand() % 0xffff;
  frame[light].color_value[1] = rand() % 0xffff;
  frame[light].color_value[2] = rand() % 0xffff;
  return true;
}

static bool light_turn_off(hue_stream_message_data *frame, int channel_count,
                           int light) {
  if (light < 0 || light >= channel_count) {
    fprintf(stderr, "light_turn_off() called with invalid light index %d\n",
            light);
    return false;
  }
  frame[light].color_value[2] = BRIGHTNESS_ZERO;
  return true;
}

#define FRAME_RATE 60
#define LIGHT_TURN_ON_INTERVAL_SECONDS 4
#define LIGHT_TURN_OFF_OR_CHANGE_INTERVAL_SECONDS 0.5

static void animate_random(hue_stream_message_data *frame, int channel_count,
                           double progress) {
  (void)progress;
  for (int i = 0; i < channel_count; i++) {
    if (light_is_on(frame, channel_count, i)) {
      if (rand() %
              (int)(LIGHT_TURN_OFF_OR_CHANGE_INTERVAL_SECONDS * FRAME_RATE) ==
          0) {
        if (rand() % 2 == 0) {
          light_turn_off(frame, channel_count, i);
        } else {
          light_to_random_color(frame, channel_count, i);
        }
      }
    } else {
      if (rand() % (int)(LIGHT_TURN_ON_INTERVAL_SECONDS * FRAME_RATE) == 0) {
        light_to_random_color(frame, channel_count, i);
      }
    }
  }
}

animation_status
animation_spider_man_into_the_spider_verse(hue_stream_message_data *frame,
                                           int channel_count,
                                           const struct timespec *start_time) {
  const animation_phase phases[] = {
      {0.0, animate_hold}, {1.0, animate_random}, {30.0, animate_hold}};

  const int num_phases = sizeof(phases) / sizeof(phases[0]);
  return animate(frame, channel_count, start_time, phases, num_phases);
}
