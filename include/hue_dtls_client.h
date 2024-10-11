#pragma once

#include "hue_stream_message.h"

typedef struct hue_dtls_context hue_dtls_context;
struct hue_dtls_context
{
};

/**
 * @brief Create a new DTLS context.
 *
 * @return A new DTLS context, or NULL on failure.
 */
hue_dtls_context *hue_dtls_context_create(void);

/**
 * @brief Free a DTLS context.
 *
 * @param context The context to free.
 */
void hue_dtls_context_free(hue_dtls_context *context);

/**
 * @brief Connect to the Hue bridge.
 *
 * Perform the DTLS handshake. Messages can be sent to the bridge using
 * @ref hue_dtls_send_message() if and only if this function returns 0.
 *
 * @param context The DTLS context.
 *
 * @return 0 on success, -1 on failure.
 */
int hue_dtls_connect(hue_dtls_context *context);

/**
 * @brief Send a message to the Hue bridge over DTLS.
 *
 * @param context The DTLS context.
 * @param message The Hue stream message to send.
 *
 * @return 0 on success, -1 on failure.
 */
int hue_dtls_send_message(hue_dtls_context *context, const hue_stream_message *message);
