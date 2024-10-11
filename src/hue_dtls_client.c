#include "hue_dtls_client.h"
#include <stddef.h> // NULL
#include <stdio.h>  // sscanf
#include <stdlib.h> // getenv, malloc, free
#include <string.h> // strncpy, strlen

#define PSK_HEX_EXPECTED_LEN 32

static unsigned int psk_client_callback(SSL *ssl, const char *hint,
                                        char *identity,
                                        unsigned int max_identity_len,
                                        unsigned char *psk,
                                        unsigned int max_psk_len) {
  const char *psk_identity = getenv("HUE_APPLICATION_ID");
  const char *psk_hex = getenv("HUE_CLIENTKEY");

  if (!psk_identity || !psk_hex) {
    return 0;
  }

  // Set the PSK identity.
  if (strlen(psk_identity) >= max_identity_len) {
    return 0;
  }

  strncpy(identity, psk_identity, max_identity_len);

  // Set the PSK key.
  const size_t psk_hex_len = strlen(psk_hex);
  if (psk_hex_len != PSK_HEX_EXPECTED_LEN) {
    return 0;
  }

  // Convert the PSK key from hex to binary.
  if (psk_hex_len / 2 > max_psk_len) {
    return 0;
  }

  for (size_t i = 0; i < psk_hex_len / 2; i++) {
    sscanf(psk_hex + 2 * i, "%2hhx", &psk[i]);
  }

  return psk_hex_len / 2;
}

hue_dtls_context *hue_dtls_context_create(void) {
  hue_dtls_context *context = malloc(sizeof(hue_dtls_context));
  if (!context) {
    return NULL;
  }

  context->ssl_ctx = SSL_CTX_new(DTLS_client_method());
  if (!context->ssl_ctx) {
    free(context);
    return NULL;
  }

  context->ssl = NULL;
  context->socket = -1;

  return context;
}

void hue_dtls_context_free(hue_dtls_context *context) {
  if (context) {
    if (context->ssl) {
      SSL_free(context->ssl);
      context->ssl = NULL;
    }

    if (context->ssl_ctx) {
      SSL_CTX_free(context->ssl_ctx);
      context->ssl_ctx = NULL;
    }

    context->socket = -1;
    free(context);
  }
}

int hue_dtls_connect(hue_dtls_context *context) { return 0; }

int hue_dtls_send_message(hue_dtls_context *context,
                          const hue_stream_message *message) {
  return 0;
}
