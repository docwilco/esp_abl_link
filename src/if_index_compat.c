/*
 * SPDX-FileCopyrightText: 2024 DocWilco
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Compatibility shim for if_nametoindex/if_indextoname.
 *
 * ESP-IDF >= 5.4 provides these in components/lwip/port/if_index.c.
 * Older versions only have lwip_if_nametoindex/lwip_if_indextoname,
 * and the ASIO component references the POSIX names directly.
 *
 * These stubs return "not found" (same approach as the official Ableton
 * Link ESP32 example). The functions are only used for IPv6 scope ID
 * resolution, which is not critical on ESP32.
 */

#include "esp_idf_version.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 4, 0)

#include <stddef.h>

unsigned int if_nametoindex(const char *ifname)
{
    (void)ifname;
    return 0;
}

char *if_indextoname(unsigned int ifindex, char *ifname)
{
    (void)ifindex;
    (void)ifname;
    return NULL;
}

#endif /* ESP_IDF_VERSION < 5.4.0 */
