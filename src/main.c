#include "animation.h"
#include "hue_dtls_client.h"
#include "hue_rest_client.h"
#include <pthread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>  // fprintf, printf, getchar
#include <stdlib.h> // free, srand
#include <string.h> // memcpy
#include <time.h>   // nanosleep

#define CHANNEL_COUNT 10
#define ENTERTAINMENT_CONFIG_ID "2d4cb563-4244-4bfc-9bb2-f5a08068df84"

#define FRAMES_PER_SECOND 60
#define NANOSECONDS_PER_FRAME (1000000000L / FRAMES_PER_SECOND)

pthread_mutex_t current_frame_mutex = {0};
hue_stream_message_data current_frame[CHANNEL_COUNT] = {0};

bool streaming = true;

typedef struct stream_thread_args stream_thread_args;
struct stream_thread_args {
  hue_dtls_context *context;
};

void *stream(void *arg) {
  const stream_thread_args *args = (stream_thread_args *)arg;

  while (streaming) {
    // Copy the current frame to minimize the time the mutex is locked.
    hue_stream_message_data frame_copy[CHANNEL_COUNT] = {0};
    pthread_mutex_lock(&current_frame_mutex);
    memcpy(frame_copy, current_frame, sizeof(current_frame));
    pthread_mutex_unlock(&current_frame_mutex);

    hue_stream_message *message = hue_stream_message_create(
        frame_copy, CHANNEL_COUNT, ENTERTAINMENT_CONFIG_ID);
    if (!message) {
      fprintf(stderr, "hue_stream_message_create() failed\n");
      return NULL;
    }

    if (hue_dtls_send_message(args->context, message, CHANNEL_COUNT)) {
      fprintf(stderr, "hue_dtls_send_message() failed\n");
      return NULL;
    }

    free(message);

    // Stream at the specified frame rate.
    struct timespec ts = {.tv_sec = 0, .tv_nsec = NANOSECONDS_PER_FRAME};
    nanosleep(&ts, NULL);
  }

  return NULL;
}

static hue_dtls_context *connect_to_bridge(const char *bridge_ip) {
  if (!bridge_ip) {
    fprintf(stderr, "bridge_ip is null\n");
    return NULL;
  }

  // Start entertainment area streaming.
  if (hue_rest_start_entertainment_area_streaming(bridge_ip,
                                                  ENTERTAINMENT_CONFIG_ID)) {
    fprintf(stderr, "hue_rest_start_entertainment_area_streaming() failed\n");
    return NULL;
  }

  // Perform the DTLS handshake.
  hue_dtls_context *context = hue_dtls_context_create();
  if (!context) {
    fprintf(stderr, "hue_dtls_context_create() failed\n");
    return NULL;
  }

  if (hue_dtls_connect(context, bridge_ip)) {
    fprintf(stderr, "hue_dtls_connect() failed\n");
    hue_dtls_context_free(context);
    return NULL;
  }

  return context;
}

static void initialize_frame(hue_stream_message_data *frame,
                             int channel_count) {
  for (int i = 0; i < channel_count; i++) {
    frame[i].channel_id = i;
    frame[i].color_value[0] = 0;
    frame[i].color_value[1] = 0;
    frame[i].color_value[2] = 0;
  }
}

volatile sig_atomic_t animating = true;

static void handle_signal(int signal) {
  if (signal == SIGINT) {
    animating = false;
  }
}

static void animate(int animation) {
  struct timespec start_time = {0};
  if (clock_gettime(CLOCK_MONOTONIC, &start_time)) {
    fprintf(stderr, "clock_gettime() failed\n");
    return;
  }

  hue_stream_message_data frame[CHANNEL_COUNT] = {0};
  initialize_frame(frame, CHANNEL_COUNT);

  animating = true;
  while (animating) {
    animation_status status = 0;
    switch (animation) {
    case ANIMATION_THX_DEEP_NOTE:
      status = animation_thx_deep_note(frame, CHANNEL_COUNT, &start_time);
      break;
    case ANIMATION_SPIDER_MAN_INTO_THE_SPIDER_VERSE:
      status = animation_spider_man_into_the_spider_verse(frame, CHANNEL_COUNT,
                                                          &start_time);
      break;
    case ANIMATION_SPIDER_MAN_ACROSS_THE_SPIDER_VERSE:
      status = animation_spider_man_across_the_spider_verse(frame, CHANNEL_COUNT,
                                                          &start_time);
      break;
    default:
      fprintf(stderr, "Invalid animation\n");
      animating = false;
      break;
    }

    if (status == ANIMATION_STATUS_ERROR) {
      fprintf(stderr, "Animation failed\n");
      animating = false;
      break;
    }

    if (status == ANIMATION_STATUS_END) {
      animating = false;
      break;
    }

    // Update the current frame.
    pthread_mutex_lock(&current_frame_mutex);
    memcpy(current_frame, frame, sizeof(frame));
    pthread_mutex_unlock(&current_frame_mutex);

    // Animate at the specified frame rate.
    struct timespec ts = {.tv_sec = 0, .tv_nsec = NANOSECONDS_PER_FRAME};
    nanosleep(&ts, NULL);
  }

  // Turn lights off after the animation ends or is interrupted.
  initialize_frame(current_frame, CHANNEL_COUNT);
}

static void display_menu() {
  while (true) {
    printf("\n--------------------------------\n");
    printf("1. THX Deep Note\n");
    printf("2. Spider-Man: Into the Spider-Verse\n");
    printf("3. Spider-Man: Across the Spider-Verse\n");
    printf("4. Quit\n");
    printf("--------------------------------\n");

    printf("Enter your choice: ");
    int choice = getchar();
    int c = 0;
    while ((c = getchar()) != '\n' && c != EOF)
      ;

    switch (choice) {
    case '1':
      animate(ANIMATION_THX_DEEP_NOTE);
      break;
    case '2':
      animate(ANIMATION_SPIDER_MAN_INTO_THE_SPIDER_VERSE);
      break;
    case '3':
      animate(ANIMATION_SPIDER_MAN_ACROSS_THE_SPIDER_VERSE);
      break;
    case '4':
      return;
    default:
      printf("Invalid choice. Please try again.\n");
      break;
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <Hue bridge IP address>\n", argv[0]);
    return 1;
  }

  const char *bridge_ip = argv[1];

  // Connect to the Hue bridge.
  printf("Connecting to Hue bridge\n");
  hue_dtls_context *context = connect_to_bridge(bridge_ip);
  if (!context) {
    fprintf(stderr, "Failed to connect to Hue bridge\n");
    return 1;
  }
  printf("Connected to Hue bridge\n");

  // Initialize the current frame.
  initialize_frame(current_frame, CHANNEL_COUNT);

  // Initialize the current frame mutex.
  if (pthread_mutex_init(&current_frame_mutex, NULL)) {
    fprintf(stderr, "pthread_mutext_init() failed\n");
    hue_dtls_context_free(context);
    return 1;
  }

  // Stream frames to the Hue bridge.
  pthread_t stream_thread = 0;
  stream_thread_args args = {context};
  if (pthread_create(&stream_thread, NULL, stream, &args)) {
    fprintf(stderr, "pthread_create() failed\n");
    pthread_mutex_destroy(&current_frame_mutex);
    hue_dtls_context_free(context);
    return 1;
  }

  // Handle Ctrl+C to stop animating.
  signal(SIGINT, handle_signal);

  // Seed the random number generator.
  const time_t seed = time(NULL);
  printf("seed: %ld\n", seed);
  srand(seed);

  // Display animation menu.
  display_menu();

  // Stop streaming.
  streaming = false;
  pthread_join(stream_thread, NULL);

  pthread_mutex_destroy(&current_frame_mutex);
  hue_dtls_context_free(context);
  return 0;
}
