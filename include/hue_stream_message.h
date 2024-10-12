#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint8_t

#define HUE_STREAM_MESSAGE_PROTOCOL_NAME_SIZE 9
#define HUE_STREAM_MESSAGE_VERSION_SIZE 2
#define HUE_STREAM_MESSAGE_RESERVED_SIZE 2
#define HUE_STREAM_MESSAGE_ENTERTAINMENT_CONFIG_ID_SIZE 36
#define HUE_STREAM_MESSAGE_MAX_CHANNELS 20

typedef struct hue_stream_message_data hue_stream_message_data;
struct hue_stream_message_data {
  uint8_t channel_id;
  uint16_t color_value[3];
};

typedef struct hue_stream_message hue_stream_message;
struct hue_stream_message {
  char protocol_name[HUE_STREAM_MESSAGE_PROTOCOL_NAME_SIZE];
  uint8_t version[HUE_STREAM_MESSAGE_VERSION_SIZE];
  uint8_t sequence_id;
  uint8_t reserved1[HUE_STREAM_MESSAGE_RESERVED_SIZE];
  uint8_t color_space;
  uint8_t reserved2;
  uint8_t
      entertainment_config_id[HUE_STREAM_MESSAGE_ENTERTAINMENT_CONFIG_ID_SIZE];
  hue_stream_message_data data[HUE_STREAM_MESSAGE_MAX_CHANNELS];
};

/**
 * @brief Serialize a Hue stream message.
 *
 * @param[in] message The message to serialize.
 * @param[in] channel_count The number of channels to serialize. The first
 * channel_count channels in the message data will be serialized.
 * @param[out] buffer The buffer to serialize the message into.
 * @param[out] buffer_size The size of the buffer.
 */
void hue_stream_message_serialize(const hue_stream_message *message,
                                  int channel_count, uint8_t **buffer,
                                  size_t *buffer_size);
