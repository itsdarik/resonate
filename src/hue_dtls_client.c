#include "hue_dtls_client.h"
#include <netinet/in.h> // struct sockaddr_in
#include <stddef.h>     // NULL, size_t
#include <stdio.h>      // perror, sscanf
#include <stdlib.h>     // getenv, malloc, free
#include <string.h>     // strncpy, strlen
#include <sys/socket.h> // socket, connect
#include <unistd.h>     // close

#define MAX_PORT_NUMBER 65535
#define PSK_HEX_EXPECTED_LEN 32

static int socket_connect(hue_dtls_context *context, const char *bridge_ip,
                          const char *bridge_port) {
  if (!context) {
    fprintf(stderr, "context is null\n");
    return -1;
  }

  const int port = atoi(bridge_port);
  if (port <= 0 || port > MAX_PORT_NUMBER) {
    fprintf(stderr, "Invalid port number: %s\n", bridge_port);
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
  addr.sin_port = htons(port);
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

int hue_dtls_connect(hue_dtls_context *context, const char *bridge_ip,
                     const char *bridge_port) {
  if (!context || !context->ssl_ctx) {
    fprintf(stderr, "context or context->ssl_ctx is null\n");
    return -1;
  }

  const int port = atoi(bridge_port);
  if (port <= 0 || port > MAX_PORT_NUMBER) {
    fprintf(stderr, "Invalid port number: %s\n", bridge_port);
    return -1;
  }

  context->ssl = SSL_new(context->ssl_ctx);
  if (!context->ssl) {
    fprintf(stderr, "SSL_new() failed\n");
    return -1;
  }

  if (socket_connect(context, bridge_ip, bridge_port)) {
    fprintf(stderr, "socket_connect() failed\n");
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
  return 0;
}
