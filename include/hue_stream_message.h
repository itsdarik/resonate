#pragma once

#include <stdint.h> // uint8_t

#define HUE_STREAM_MESSAGE_PROTOCOL_NAME_SIZE 9
#define HUE_STREAM_MESSAGE_VERSION_SIZE 2
#define HUE_STREAM_MESSAGE_RESERVED_SIZE 2
#define HUE_STREAM_MESSAGE_ENTERTAINMENT_CONFIG_ID_SIZE 36
#define HUE_STREAM_MESSAGE_MAX_CHANNELS 20

typedef struct hue_stream_message_data hue_stream_message_data;
struct hue_stream_message_data
{
    uint8_t channel_id;
    uint16_t color_value[3];
};

typedef struct hue_stream_message hue_stream_message;
struct hue_stream_message
{
    char protocol_name[HUE_STREAM_MESSAGE_PROTOCOL_NAME_SIZE];
    uint8_t version[HUE_STREAM_MESSAGE_VERSION_SIZE];
    uint8_t sequence_id;
    uint8_t reserved1[HUE_STREAM_MESSAGE_RESERVED_SIZE];
    uint8_t color_space;
    uint8_t reserved2;
    uint8_t entertainment_config_id[HUE_STREAM_MESSAGE_ENTERTAINMENT_CONFIG_ID_SIZE];
    hue_stream_message_data data[HUE_STREAM_MESSAGE_MAX_CHANNELS];
};
