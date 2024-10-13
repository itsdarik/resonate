#include "hue_rest_client.h"

#include <curl/curl.h>

#include <stdlib.h> // getenv

static size_t write_callback(void *ptr, size_t size, size_t nmemb,
                             void *stream) {
  (void)ptr;
  (void)stream;
  return size * nmemb;
}

int hue_rest_start_entertainment_area_streaming(
    const char *bridge_ip, const char *entertainment_config_id) {
  // Validate input parameters.
  if (!bridge_ip || !entertainment_config_id) {
    fprintf(stderr, "bridge_ip or entertainment_config_id is null\n");
    return -1;
  }

  const char *hue_username = getenv("HUE_USERNAME");
  if (!hue_username) {
    fprintf(stderr, "HUE_USERNAME not set\n");
    return -1;
  }

  // Initialize curl.
  curl_global_init(CURL_GLOBAL_DEFAULT);
  CURL *curl = curl_easy_init();
  if (!curl) {
    fprintf(stderr, "curl_easy_init() failed\n");
    curl_global_cleanup();
    return -1;
  }

  // Set the REST API URL.
  char url[256] = {0};
  snprintf(url, sizeof(url),
           "https://%s/clip/v2/resource/entertainment_configuration/%s",
           bridge_ip, entertainment_config_id);
  curl_easy_setopt(curl, CURLOPT_URL, url);

  // Ignore SSL certificate verification.
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  // Set the request method.
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");

  // Set the request body.
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"action\":\"start\"}");

  // The request body is JSON.
  struct curl_slist *headers =
      curl_slist_append(NULL, "Content-Type: application/json");
  if (!headers) {
    fprintf(stderr, "curl_slist_append() failed\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return -1;
  }

  // Set the Hue application key.
  char hue_application_key[256] = {0};
  snprintf(hue_application_key, sizeof(hue_application_key),
           "hue-application-key: %s", hue_username);
  headers = curl_slist_append(headers, hue_application_key);
  if (!headers) {
    fprintf(stderr, "curl_slist_append() failed\n");
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return -1;
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

  // Don't write the response to stdout.
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

  int ret = 0;

  // Perform the REST API request.
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    fprintf(stderr, "curl_easy_perform() failed: %s\n",
            curl_easy_strerror(res));
    ret = -1;
  }

  curl_slist_free_all(headers);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
  return ret;
}
