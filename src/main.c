#include "hue_dtls_client.h"
#include <stdio.h>

#define HUE_BRIDGE_IP_ADDRESS "192.168.50.24"

int main(void) {
  hue_dtls_context *context = hue_dtls_context_create();
  if (!context) {
    fprintf(stderr, "Failed to create DTLS context\n");
    return 1;
  }

  if (hue_dtls_connect(context, HUE_BRIDGE_IP_ADDRESS)) {
    fprintf(stderr, "Failed to connect to Hue bridge\n");
    hue_dtls_context_free(context);
    return 1;
  }

  hue_dtls_context_free(context);
  return 0;
}
