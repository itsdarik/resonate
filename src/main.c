#include <stdio.h>
#include "hue_dtls_client.h"

int main(void)
{
    hue_dtls_context *context = hue_dtls_context_create();
    if (!context)
    {
        fprintf(stderr, "Failed to create DTLS context\n");
        return 1;
    }

    if (hue_dtls_connect(context))
    {
        fprintf(stderr, "Failed to connect to Hue bridge\n");
        hue_dtls_context_free(context);
        return 1;
    }

    hue_dtls_context_free(context);
    return 0;
}
