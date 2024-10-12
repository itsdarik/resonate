#include "hue_stream_message.h"

#include <stdio.h>  // fprintf, perror
#include <stdlib.h> // malloc
#include <string.h> // memcpy

hue_stream_message *
hue_stream_message_create(const hue_stream_message_data *data,
                          int channel_count) {
  if (!data) {
    fprintf(stderr, "data is null\n");
    return NULL;
  }

  if (!hue_stream_message_valid_channel_count(channel_count)) {
    fprintf(stderr, "channel_count is out of range\n");
    return NULL;
  }
}

void hue_stream_message_serialize(const hue_stream_message *message,
                                  int channel_count, uint8_t **buffer,
                                  size_t *buffer_size) {
  if (!message || !buffer || !buffer_size) {
    fprintf(stderr, "message, buffer, or buffer_size is null\n");
    return;
  }

  if (!hue_stream_message_valid_channel_count(channel_count)) {
    fprintf(stderr, "channel_count is out of range\n");
    return;
  }

  *buffer_size = sizeof(message->protocol_name) + sizeof(message->version) +
                 sizeof(message->sequence_id) + sizeof(message->reserved1) +
                 sizeof(message->color_space) + sizeof(message->reserved2) +
                 sizeof(message->entertainment_config_id) +
                 channel_count * (sizeof(message->data[0].channel_id) +
                                  sizeof(message->data[0].color_value));

  *buffer = malloc(*buffer_size);
  if (!*buffer) {
    perror("malloc");
    return;
  }

  uint8_t *ptr = *buffer;

  memcpy(ptr, message->protocol_name, sizeof(message->protocol_name));
  ptr += sizeof(message->protocol_name);

  memcpy(ptr, message->version, sizeof(message->version));
  ptr += sizeof(message->version);

  memcpy(ptr, &message->sequence_id, sizeof(message->sequence_id));
  ptr += sizeof(message->sequence_id);

  memcpy(ptr, message->reserved1, sizeof(message->reserved1));
  ptr += sizeof(message->reserved1);

  memcpy(ptr, &message->color_space, sizeof(message->color_space));
  ptr += sizeof(message->color_space);

  memcpy(ptr, &message->reserved2, sizeof(message->reserved2));
  ptr += sizeof(message->reserved2);

  memcpy(ptr, message->entertainment_config_id,
         sizeof(message->entertainment_config_id));
  ptr += sizeof(message->entertainment_config_id);

  for (int i = 0; i < channel_count; i++) {
    memcpy(ptr, &message->data[i].channel_id,
           sizeof(message->data[i].channel_id));
    ptr += sizeof(message->data[i].channel_id);

    memcpy(ptr, message->data[i].color_value,
           sizeof(message->data[i].color_value));
    ptr += sizeof(message->data[i].color_value);
  }
}

bool hue_stream_message_valid_channel_count(int channel_count) {
  return channel_count >= 0 && channel_count <= HUE_STREAM_MESSAGE_MAX_CHANNELS;
}
