#pragma once

#include <stdbool.h> // bool
#include <stddef.h>  // size_t
#include <stdint.h>  // uint8_t

#define HUE_STREAM_MESSAGE_PROTOCOL_NAME_SIZE 9
#define HUE_STREAM_MESSAGE_VERSION_SIZE 2
#define HUE_STREAM_MESSAGE_RESERVED_SIZE 2
#define HUE_STREAM_MESSAGE_ENTERTAINMENT_CONFIG_ID_SIZE 36
#define HUE_STREAM_MESSAGE_MAX_CHANNELS 20
#define HUE_STREAM_MESSAGE_COLOR_VALUE_ELEMENTS 3

#define HUE_STREAM_MESSAGE_COLOR_SPACE_RGB 0x00
#define HUE_STREAM_MESSAGE_COLOR_SPACE_XY_BRIGHTNESS 0x01

typedef struct hue_stream_message_data hue_stream_message_data;
struct hue_stream_message_data {
  uint8_t channel_id;
  uint16_t color_value[HUE_STREAM_MESSAGE_COLOR_VALUE_ELEMENTS];
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
 * @brief Create a new Hue stream message.
 *
 * The user is responsible for freeing the message.
 *
 * @param[in] data The message data array in xy + brightness color space.
 * @param[in] channel_count The length of the data array.
 * @param[in] entertainment_config_id The entertainment configuration ID.
 *
 * @return A new Hue stream message, or NULL on failure.
 */
hue_stream_message *
hue_stream_message_create(const hue_stream_message_data *data,
                          int channel_count,
                          const char *entertainment_config_id);

/**
 * @brief Serialize a Hue stream message.
 *
 * The user is responsible for freeing the buffer.
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

/**
 * @brief Determine if a channel count is valid.
 *
 * @param[in] channel_count The channel count to validate.
 *
 * @return true if the channel count is valid, false otherwise.
 */
bool hue_stream_message_valid_channel_count(int channel_count);
