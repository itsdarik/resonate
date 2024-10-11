#include <stddef.h> // NULL
#include "hue_dtls_client.h"

hue_dtls_context *hue_dtls_context_create(void)
{
    return NULL;
}

void hue_dtls_context_free(hue_dtls_context *context)
{
    return;
}

int hue_dtls_connect(hue_dtls_context *context)
{
    return 0;
}

int hue_dtls_send_message(hue_dtls_context *context, const hue_stream_message *message)
{
    return 0;
}
