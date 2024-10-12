#include "hue_dtls_client.h"
#include <stdbool.h> // bool
#include <stdio.h>   // fprintf, printf
#include <unistd.h>  // sleep

#define CHANNEL_COUNT 10
#define ENTERTAINMENT_CONFIG_ID "2d4cb563-4244-4bfc-9bb2-f5a08068df84"

bool streaming = true;

hue_stream_message_data current_frame[CHANNEL_COUNT] = {
    {0, {0x0000, 0xFFFF, 0xFFFF}}, {1, {0x0000, 0xFFFF, 0xFFFF}},
    {2, {0x0000, 0xFFFF, 0xFFFF}}, {3, {0x0000, 0xFFFF, 0xFFFF}},
    {4, {0x0000, 0xFFFF, 0xFFFF}}, {5, {0x0000, 0xFFFF, 0xFFFF}},
    {6, {0x0000, 0xFFFF, 0xFFFF}}, {7, {0x0000, 0xFFFF, 0xFFFF}},
    {8, {0x0000, 0xFFFF, 0xFFFF}}, {9, {0x0000, 0xFFFF, 0xFFFF}},
};

typedef struct stream_thread_args stream_thread_args;
struct stream_thread_args {
  hue_dtls_context *context;
};

void *stream(void *arg) {
  const stream_thread_args *args = (stream_thread_args *)arg;

  while (streaming) {
    const hue_stream_message *message = hue_stream_message_create(
        current_frame, CHANNEL_COUNT, ENTERTAINMENT_CONFIG_ID);
    if (!message) {
      fprintf(stderr, "hue_stream_message_create() failed\n");
      return NULL;
    }

    if (hue_dtls_send_message(args->context, message, CHANNEL_COUNT)) {
      fprintf(stderr, "hue_dtls_send_message() failed\n");
      return NULL;
    }

    hue_stream_message_free(message);
  }

  return NULL;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <Hue bridge IP address>\n", argv[0]);
    return 1;
  }

  const char *bridge_ip = argv[1];

  // Connect to the Hue bridge.
  hue_dtls_context *context = hue_dtls_context_create();
  if (!context) {
    fprintf(stderr, "Failed to create DTLS context\n");
    return 1;
  }

  printf("Connecting to Hue bridge\n");

  if (hue_dtls_connect(context, bridge_ip)) {
    fprintf(stderr, "Failed to connect to Hue bridge\n");
    hue_dtls_context_free(context);
    return 1;
  }

  printf("Connected to Hue bridge\n");

  // Stream frames to the Hue bridge around 60 frames per second.
  pthread_t stream_thread = 0;
  stream_thread_args args = {context};
  if (pthread_create(&stream_thread, NULL, stream, &args)) {
    fprintf(stderr, "pthread_create() failed\n");
    hue_dtls_context_free(context);
    return 1;
  }

  sleep(30);

  // Stop streaming.
  streaming = false;
  pthread_join(stream_thread, NULL);

  hue_dtls_context_free(context);
  return 0;
}

/*
 * TODO:
 * 3. Create a thread to send the current state to the Hue bridge at 60 Hz.
 * 4. Create a menu to select an animation.
 * 5. Implement the animations on the main thread at 60 Hz.
 * 6. Stop the animation on a Ctrl+C interrupt signal, return to the menu.
 * 7. Quit or Ctrl+D on the menu stops the sender thread and exits the program.
 */
