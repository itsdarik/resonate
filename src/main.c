#include "hue_dtls_client.h"
#include <pthread.h>
#include <stdbool.h> // bool
#include <stdio.h>   // fprintf, printf, scanf
#include <stdlib.h>  // free
#include <string.h>  // memcpy
#include <time.h>    // nanosleep
#include <unistd.h>  // sleep

#define CHANNEL_COUNT 10
#define ENTERTAINMENT_CONFIG_ID "2d4cb563-4244-4bfc-9bb2-f5a08068df84"

#define FRAMES_PER_SECOND 60
#define NANOSECONDS_PER_FRAME (1000000000L / FRAMES_PER_SECOND)

bool streaming = true;

pthread_mutex_t current_frame_mutex = {0};
hue_stream_message_data current_frame[CHANNEL_COUNT] = {0};

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

static void display_menu() {
  while (true) {
    printf("--------------------------------\n");
    printf("1. THX Deep Note\n");
    printf("2. Spider-Man: Into the Spider-Verse\n");
    printf("3. Quit\n");
    printf("--------------------------------\n");

    printf("Enter your choice: ");
    int choice = 0;
    scanf("%d", &choice);

    switch (choice) {
    case 1:
      break;
    case 2:
      break;
    case 3:
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
  for (int i = 0; i < CHANNEL_COUNT; i++) {
    current_frame[i].channel_id = i;
    current_frame[i].color_value[0] = 0;
    current_frame[i].color_value[1] = 0;
    current_frame[i].color_value[2] = 0;
  }

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

  // Display animation menu.
  display_menu();

  // Stop streaming.
  streaming = false;
  pthread_join(stream_thread, NULL);

  pthread_mutex_destroy(&current_frame_mutex);
  hue_dtls_context_free(context);
  return 0;
}

/*
 * TODO:
 * - Implement animations on the main thread at 60 Hz.
 * - Ctrl+C to stop animation, Ctrl+D to exit menu.
 * - Curl to send start header.
 */
