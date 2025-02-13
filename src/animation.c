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
#define BRIGHTNESS_LOW 0x00ff
#define BRIGHTNESS_HALF 0x7fff
#define BRIGHTNESS_MAX 0xffff

#define COLOR_BLUE_X 0x2b00
#define COLOR_BLUE_Y 0x2b00

#define COLOR_WHITE_X 0x50d2
#define COLOR_WHITE_Y 0x54a9

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
               interpolate(BRIGHTNESS_ZERO, BRIGHTNESS_MAX, progress));
}

static void animate_fade_to_dim(hue_stream_message_data *frame,
                                int channel_count, double progress) {
  set_all_same_brightness(
      frame, channel_count,
      interpolate(BRIGHTNESS_MAX, BRIGHTNESS_LOW, progress));
}

static void animate_fade_to_white(hue_stream_message_data *frame,
                                  int channel_count, double progress) {
  set_all_same(frame, channel_count,
               interpolate(COLOR_BLUE_X, COLOR_WHITE_X, progress),
               interpolate(COLOR_BLUE_Y, COLOR_WHITE_Y, progress),
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
      {6.3, animate_hold},           {16.5, animate_fade_to_dim},
      {19.0, animate_fade_to_white}, {21.8, animate_hold},
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

static void animate_random_across(hue_stream_message_data *frame,
                                  int channel_count, double progress);
static void animate_black(hue_stream_message_data *frame, int channel_count,
                          double progress);

animation_status
animation_spider_man_into_the_spider_verse(hue_stream_message_data *frame,
                                           int channel_count,
                                           const struct timespec *start_time) {
  const animation_phase phases[] = {
      {0.000, animate_hold},   {6.048, animate_random_across},
      {7.049, animate_black},  {7.716, animate_random_across},
      {8.926, animate_black},  {9.509, animate_random_across},
      {10.260, animate_black}, {10.469, animate_random_across},
      {11.470, animate_black}, {11.721, animate_random_across},
      {13.764, animate_black}, {14.390, animate_random_across},
      {27.486, animate_black}, {28.112, animate_random_across},
      {29.197, animate_black}, {29.405, animate_random_across},
      {31.532, animate_black}, {32.200, animate_random_across},
      {36.536, animate_black}, {36.620, animate_random_across},
      {36.828, animate_black}, {36.954, animate_random_across},
      {44.337, animate_hold}};

  const int num_phases = sizeof(phases) / sizeof(phases[0]);
  return animate(frame, channel_count, start_time, phases, num_phases);
}

static bool lights_to_random_color(hue_stream_message_data *frame,
                                   int channel_count, int light) {
  const uint16_t x = rand() % 0xffff;
  const uint16_t y = rand() % 0xffff;
  const uint16_t brightness = 0xffff;

  for (int i = light; i < channel_count; i++) {
    frame[i].color_value[0] = x;
    frame[i].color_value[1] = y;
    frame[i].color_value[2] = brightness;
  }

  return true;
}

static bool lights_to_random_colors(hue_stream_message_data *frame,
                                    int channel_count, int start) {
  for (int i = start; i < channel_count; i++) {
    frame[i].color_value[0] = rand() % 0xffff;
    frame[i].color_value[1] = rand() % 0xffff;
    frame[i].color_value[2] = 0xffff;
  }
  return true;
}

#define ACROSS_LIGHTS_CHANGE_INTERVAL_SECONDS 0.25

static void animate_random_across(hue_stream_message_data *frame,
                                  int channel_count, double progress) {
  (void)progress;

  if (channel_count == 0) {
    return;
  }

  const bool lights_on = light_is_on(frame, channel_count, 0);

  if (lights_on) {
    if (rand() % (int)(ACROSS_LIGHTS_CHANGE_INTERVAL_SECONDS * FRAME_RATE) ==
        0) {
      if (rand() % 2 == 0) {
        lights_to_random_color(frame, channel_count, 0);
      } else {
        lights_to_random_colors(frame, channel_count, 0);
      }
    }
  } else {
    if (rand() % 2 == 0) {
      lights_to_random_color(frame, channel_count, 0);
    } else {
      lights_to_random_colors(frame, channel_count, 0);
    }
  }
}

static void animate_black(hue_stream_message_data *frame, int channel_count,
                          double progress) {
  (void)progress;
  for (int i = 0; i < channel_count; i++) {
    light_turn_off(frame, channel_count, i);
  }
}

animation_status animation_spider_man_across_the_spider_verse(
    hue_stream_message_data *frame, int channel_count,
    const struct timespec *start_time) {
  const animation_phase phases[] = {
      {0.000, animate_hold},   {8.718, animate_random_across},
      {9.260, animate_black},  {9.510, animate_random_across},
      {9.885, animate_black},  {11.053, animate_random_across},
      {13.264, animate_black}, {13.472, animate_random_across},
      {17.017, animate_black}, {17.852, animate_random_across},
      {17.935, animate_black}, {18.436, animate_random_across},
      {18.686, animate_black}, {19.145, animate_random_across},
      {19.478, animate_black}, {20.229, animate_random_across},
      {20.813, animate_black}, {21.272, animate_random_across},
      {22.398, animate_black}, {22.481, animate_random_across},
      {22.898, animate_black}, {24.483, animate_random_across},
      {25.651, animate_black}, {26.986, animate_random_across},
      {35.995, animate_black}, {37.788, animate_random_across},
      {38.706, animate_black}, {38.748, animate_random_across},
      {43.169, animate_black}, {43.461, animate_random_across},
      {43.502, animate_black}, {44.837, animate_random_across},
      {46.505, animate_black}, {47.173, animate_random_across},
      {53.763, animate_black}, {54.096, animate_random_across},
      {54.638, animate_black}, {54.889, animate_random_across},
      {56.140, animate_black}, {56.265, animate_random_across},
      {56.599, animate_black}, {56.807, animate_random_across},
      {57.641, animate_black}, {58.642, animate_random_across},
      {58.893, animate_black}, {59.185, animate_random_across},
      {61.228, animate_hold}};

  const int num_phases = sizeof(phases) / sizeof(phases[0]);
  return animate(frame, channel_count, start_time, phases, num_phases);
}
