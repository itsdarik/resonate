#include "hue_dtls_client.h"
#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <Hue bridge IP address>\n", argv[0]);
    return 1;
  }

  const char *bridge_ip = argv[1];

  hue_dtls_context *context = hue_dtls_context_create();
  if (!context) {
    fprintf(stderr, "Failed to create DTLS context\n");
    return 1;
  }

  printf("Connecting to Hue bridge\n");

  if (hue_dtls_connect(context, bridge_ip)) {
    fprintf(stderr, "Failed to connect to Hue bridge\n");
    hue_dtls_context_free(context);
    return 1;
  }

  printf("Connected to Hue bridge\n");

  hue_dtls_context_free(context);
  return 0;
}
