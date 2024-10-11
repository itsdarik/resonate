#include "hue_dtls_client.h"
#include <stddef.h> // NULL

static unsigned int psk_client_callback(SSL *ssl, const char *hint,
                                        char *identity,
                                        unsigned int max_identity_len,
                                        unsigned char *psk,
                                        unsigned int max_psk_len) {
  return 0;
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
