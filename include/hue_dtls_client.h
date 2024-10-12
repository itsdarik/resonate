#pragma once

#include "hue_stream_message.h"
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/net_sockets.h>
#include <mbedtls/ssl.h>

typedef struct hue_dtls_context hue_dtls_context;
struct hue_dtls_context {
  mbedtls_net_context server_fd;
  mbedtls_ssl_context ssl;
  mbedtls_ssl_config conf;
  mbedtls_ctr_drbg_context ctr_drbg;
  mbedtls_entropy_context entropy;
};

/**
 * @brief Create a new DTLS context.
 *
 * This function requires these environment variables to be set:
 * - HUE_APPLICATION_ID
 * - HUE_CLIENTKEY
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
 * @ref hue_dtls_send_message() if this function returns 0.
 *
 * @param context The DTLS context.
 * @param bridge_ip The IP address of the Hue bridge.
 *
 * @return 0 on success, -1 on failure.
 */
int hue_dtls_connect(hue_dtls_context *context, const char *bridge_ip);

/**
 * @brief Send a message to the Hue bridge over DTLS.
 *
 * @param context The DTLS context.
 * @param message The Hue stream message to send.
 *
 * @return 0 on success, -1 on failure.
 */
int hue_dtls_send_message(hue_dtls_context *context,
                          const hue_stream_message *message);
