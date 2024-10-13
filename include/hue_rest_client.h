#pragma once

/**
 * @brief Start the entertainment area streaming.
 *
 * This function requires these environment variables to be set:
 * - HUE_USERNAME
 *
 * @param bridge_ip The IP address of the Hue bridge.
 * @param entertainment_config_id The entertainment configuration ID.
 *
 * @return 0 on success, -1 on failure.
 */
int hue_rest_start_entertainment_area_streaming(
    const char *bridge_ip, const char *entertainment_config_id);
