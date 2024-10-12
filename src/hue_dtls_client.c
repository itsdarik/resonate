#include "hue_dtls_client.h"
#include <arpa/inet.h>  // inet_pton
#include <errno.h>      // errno
#include <netinet/in.h> // struct sockaddr_in
#include <stddef.h>     // NULL, size_t
#include <stdio.h>      // perror, sscanf
#include <stdlib.h>     // strtol, getenv, malloc, free
#include <string.h>     // strncpy, strlen
#include <sys/socket.h> // socket, connect
#include <unistd.h>     // close

#define HUE_BRIDGE_DTLS_CIPHER "PSK-AES128-GCM-SHA256"
#define HUE_BRIDGE_DTLS_PORT 2100
#define PSK_HEX_EXPECTED_LEN 32

static int udp_socket_connect(hue_dtls_context *context,
                              const char *bridge_ip) {
  if (!context) {
    fprintf(stderr, "context is null\n");
    return -1;
  }

  context->socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (context->socket < 0) {
    perror("socket");
    context->socket = -1;
    return -1;
  }

  struct sockaddr_in addr = {0};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(HUE_BRIDGE_DTLS_PORT);
  if (inet_pton(AF_INET, bridge_ip, &addr.sin_addr) != 1) {
    perror("inet_pton");
    close(context->socket);
    context->socket = -1;
    return -1;
  }

  if (connect(context->socket, (struct sockaddr *)&addr, sizeof(addr))) {
    perror("connect");
    close(context->socket);
    context->socket = -1;
    return -1;
  }

  return 0;
}

static unsigned int psk_client_callback(SSL *ssl, const char *hint,
                                        char *identity,
                                        unsigned int max_identity_len,
                                        unsigned char *psk,
                                        unsigned int max_psk_len) {
  // Suppress unused parameter warnings.
  (void)ssl;
  (void)hint;

  const char *psk_identity = getenv("HUE_APPLICATION_ID");
  const char *psk_hex = getenv("HUE_CLIENTKEY");

  if (!psk_identity || !psk_hex) {
    fprintf(stderr, "HUE_APPLICATION_ID or HUE_CLIENTKEY not set\n");
    return 0;
  }

  // Set the PSK identity.
  if (strlen(psk_identity) >= max_identity_len) {
    fprintf(stderr, "PSK identity is too long\n");
    return 0;
  }

  strncpy(identity, psk_identity, max_identity_len);

  // Set the PSK key.
  const size_t psk_hex_len = strlen(psk_hex);
  if (psk_hex_len != PSK_HEX_EXPECTED_LEN) {
    fprintf(stderr, "PSK hex length (%zu) is not the expected length (%d)\n",
            psk_hex_len, PSK_HEX_EXPECTED_LEN);
    return 0;
  }

  // Convert the PSK key from hex to binary.
  if (psk_hex_len / 2 > max_psk_len) {
    fprintf(stderr, "PSK hex is too long\n");
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
    perror("malloc");
    return NULL;
  }

  context->ssl_ctx = SSL_CTX_new(DTLS_client_method());
  if (!context->ssl_ctx) {
    fprintf(stderr, "SSL_CTX_new() failed\n");
    free(context);
    return NULL;
  }

  SSL_CTX_set_psk_client_callback(context->ssl_ctx, psk_client_callback);

  if (SSL_CTX_set_cipher_list(context->ssl_ctx, HUE_BRIDGE_DTLS_CIPHER) != 1) {
    fprintf(stderr, "SSL_CTX_set_cipher_list() failed\n");
    SSL_CTX_free(context->ssl_ctx);
    context->ssl_ctx = NULL;
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

    if (context->socket != -1) {
      close(context->socket);
      context->socket = -1;
    }

    free(context);
  }
}

int hue_dtls_connect(hue_dtls_context *context, const char *bridge_ip) {
  if (!context || !context->ssl_ctx) {
    fprintf(stderr, "context or context->ssl_ctx is null\n");
    return -1;
  }

  context->ssl = SSL_new(context->ssl_ctx);
  if (!context->ssl) {
    fprintf(stderr, "SSL_new() failed\n");
    return -1;
  }

  if (udp_socket_connect(context, bridge_ip)) {
    fprintf(stderr, "udp_socket_connect() failed\n");
    SSL_free(context->ssl);
    context->ssl = NULL;
    return -1;
  }

  if (SSL_set_fd(context->ssl, context->socket) != 1) {
    fprintf(stderr, "SSL_set_fd() failed\n");
    SSL_free(context->ssl);
    context->ssl = NULL;
    close(context->socket);
    context->socket = -1;
    return -1;
  }

  if (SSL_connect(context->ssl) != 1) {
    fprintf(stderr, "SSL_connect() failed\n");
    SSL_free(context->ssl);
    context->ssl = NULL;
    close(context->socket);
    context->socket = -1;
    return -1;
  }

  return 0;
}

int hue_dtls_send_message(hue_dtls_context *context,
                          const hue_stream_message *message) {
  if (!context || !context->ssl || !message) {
    fprintf(stderr, "context, context->ssl, or message is null\n");
    return -1;
  }

  return 0;
}
