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

#include <mbedtls/debug.h>
#include <mbedtls/timing.h>

#define HUE_BRIDGE_DTLS_CIPHER "TLS-PSK-WITH-AES-128-GCM-SHA256"
#define HUE_BRIDGE_DTLS_PORT 2100
#define PSK_HEX_EXPECTED_LEN 32

typedef struct {
    mbedtls_timing_delay_context timer;
} dtls_timer_context;

static void timer_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms) {
    dtls_timer_context *ctx = (dtls_timer_context *)data;
    mbedtls_timing_set_delay(&ctx->timer, int_ms, fin_ms);
}

static int timer_get_delay(void *data) {
    dtls_timer_context *ctx = (dtls_timer_context *)data;
    return mbedtls_timing_get_delay(&ctx->timer);
}

static int udp_socket_connect(hue_dtls_context *context, const char *bridge_ip) {
    if (!context) {
        fprintf(stderr, "context is null\n");
        return -1;
    }

    mbedtls_net_init(&context->net);

    if (mbedtls_net_connect(&context->net, bridge_ip, "2100", MBEDTLS_NET_PROTO_UDP) != 0) {
        fprintf(stderr, "mbedtls_net_connect() failed\n");
        return -1;
    }

    return 0;
}

static int setup_psk(mbedtls_ssl_config *conf) {
    const char *psk_identity = getenv("HUE_APPLICATION_ID");
    const char *psk_hex = getenv("HUE_CLIENTKEY");

    if (!psk_identity || !psk_hex) {
        fprintf(stderr, "HUE_APPLICATION_ID or HUE_CLIENTKEY not set\n");
        return -1;
    }

    unsigned char psk[PSK_HEX_EXPECTED_LEN / 2];
    const size_t psk_hex_len = strlen(psk_hex);

    if (psk_hex_len != PSK_HEX_EXPECTED_LEN) {
        fprintf(stderr, "PSK hex length (%zu) is not the expected length (%d)\n", psk_hex_len, PSK_HEX_EXPECTED_LEN);
        return -1;
    }

    for (size_t i = 0; i < psk_hex_len / 2; i++) {
        if (sscanf(psk_hex + 2 * i, "%2hhx", &psk[i]) != 1) {
            fprintf(stderr, "Failed to parse PSK hex string\n");
            return -1;
        }
    }

    if (mbedtls_ssl_conf_psk(conf, psk, sizeof(psk), (const unsigned char *)psk_identity, strlen(psk_identity)) != 0) {
        fprintf(stderr, "mbedtls_ssl_conf_psk() failed\n");
        return -1;
    }

    return 0;
}

hue_dtls_context *hue_dtls_context_create(void) {
    hue_dtls_context *context = malloc(sizeof(hue_dtls_context));
    if (!context) {
        perror("malloc");
        return NULL;
    }

    mbedtls_ssl_init(&context->ssl);
    mbedtls_ssl_config_init(&context->conf);
    mbedtls_ctr_drbg_init(&context->ctr_drbg);
    mbedtls_entropy_init(&context->entropy);

    if (mbedtls_ctr_drbg_seed(&context->ctr_drbg, mbedtls_entropy_func, &context->entropy, NULL, 0) != 0) {
        fprintf(stderr, "mbedtls_ctr_drbg_seed() failed\n");
        free(context);
        return NULL;
    }

    if (mbedtls_ssl_config_defaults(&context->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_DATAGRAM, MBEDTLS_SSL_PRESET_DEFAULT) != 0) {
        fprintf(stderr, "mbedtls_ssl_config_defaults() failed\n");
        free(context);
        return NULL;
    }

    mbedtls_ssl_conf_rng(&context->conf, mbedtls_ctr_drbg_random, &context->ctr_drbg);

    if (setup_psk(&context->conf) != 0) {
        free(context);
        return NULL;
    }

    if (mbedtls_ssl_setup(&context->ssl, &context->conf) != 0) {
        fprintf(stderr, "mbedtls_ssl_setup() failed\n");
        free(context);
        return NULL;
    }

    return context;
}

void hue_dtls_context_free(hue_dtls_context *context) {
    if (context) {
        mbedtls_ssl_free(&context->ssl);
        mbedtls_ssl_config_free(&context->conf);
        mbedtls_ctr_drbg_free(&context->ctr_drbg);
        mbedtls_entropy_free(&context->entropy);
        mbedtls_net_free(&context->net);
        free(context);
    }
}

void my_debug(void *ctx, int level, const char *file, int line, const char *str) {
    ((void) level);
    fprintf((FILE *) ctx, "%s:%04d: %s", file, line, str);
}

int hue_dtls_connect(hue_dtls_context *context, const char *bridge_ip) {
    if (!context) {
        fprintf(stderr, "context is null\n");
        return -1;
    }

    if (udp_socket_connect(context, bridge_ip) != 0) {
        fprintf(stderr, "udp_socket_connect() failed\n");
        return -1;
    }

    mbedtls_debug_set_threshold(4); // Set debug level (0-4)
    mbedtls_ssl_conf_dbg(&context->conf, my_debug, stderr);

    // Set up timer callbacks
    dtls_timer_context timer_ctx;
    mbedtls_ssl_set_timer_cb(&context->ssl, &timer_ctx, timer_set_delay, timer_get_delay);

    mbedtls_ssl_set_bio(&context->ssl, &context->net, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

    int ret;
    while ((ret = mbedtls_ssl_handshake(&context->ssl)) != 0) {
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            // Non-fatal error, retry the handshake
            continue;
        } else {
            fprintf(stderr, "mbedtls_ssl_handshake() failed: -0x%x\n", -ret);
            return -1;
        }
    }

    return 0;
}

int hue_dtls_send_message(hue_dtls_context *context, const hue_stream_message *message) {
    if (!context || !message) {
        fprintf(stderr, "context or message is null\n");
        return -1;
    }

    // Implement message sending logic here

    return 0;
}
