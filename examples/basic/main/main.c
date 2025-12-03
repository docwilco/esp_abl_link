/*
 * Ableton Link ESP32 Example
 *
 * Based on the Ableton Link ESP32 example and abl_link C wrapper example.
 * Original examples: https://github.com/Ableton/link/tree/master/examples/esp32
 *                    https://github.com/Ableton/link/tree/master/extensions/abl_link/examples/link_hut
 *
 * Copyright 2020, Ableton AG, Berlin. All rights reserved.
 * Copyright 2019, Mathias Bredholt, Torso Electronics, Copenhagen. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"

#include "abl_link.h"

static const char *TAG = "link_example";

// LED GPIO - change this to match your board
// GPIO 2 is the built-in LED on many ESP32 dev boards
#define LED_GPIO GPIO_NUM_2

// Set to false to disable printing Link state to console
#define PRINT_LINK_STATE true

// Quantum (beats per bar) for phase calculations
#define QUANTUM 4.0

typedef struct {
    abl_link link;
    abl_link_session_state session_state;
} link_state_t;

static void print_task(void *user_param)
{
    link_state_t *state = (link_state_t *)user_param;

    while (true) {
        abl_link_capture_app_session_state(state->link, state->session_state);

        const uint64_t time = abl_link_clock_micros(state->link);
        const size_t num_peers = abl_link_num_peers(state->link);
        const double tempo = abl_link_tempo(state->session_state);
        const double beats = abl_link_beat_at_time(state->session_state, time, QUANTUM);
        const double phase = abl_link_phase_at_time(state->session_state, time, QUANTUM);
        const bool is_playing = abl_link_is_playing(state->session_state);

        ESP_LOGI(TAG, "peers: %zu | tempo: %.1f | beats: %.2f | phase: %.2f | %s",
                 num_peers, tempo, beats, phase,
                 is_playing ? "playing" : "stopped");

        vTaskDelay(pdMS_TO_TICKS(800));
    }
}

static void tick_task(void *user_param)
{
    link_state_t *state = (link_state_t *)user_param;

    // Configure LED GPIO
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    while (true) {
        abl_link_capture_audio_session_state(state->link, state->session_state);

        const uint64_t time = abl_link_clock_micros(state->link);
        const double phase = abl_link_phase_at_time(state->session_state, time, 1.0);

        // Flash LED on each beat (LED on for first 10% of beat)
        gpio_set_level(LED_GPIO, fmod(phase, 1.0) < 0.1 ? 1 : 0);

        vTaskDelay(1);
    }
}

void app_main(void)
{
    // Initialize NVS (required for WiFi)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize networking
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to WiFi (uses settings from menuconfig)
    ESP_ERROR_CHECK(example_connect());

    ESP_LOGI(TAG, "WiFi connected, starting Ableton Link...");

    // Initialize Link state
    static link_state_t state;
    state.link = abl_link_create(120.0);  // Initial tempo: 120 BPM
    state.session_state = abl_link_create_session_state();

    // Enable Link
    abl_link_enable(state.link, true);
    ESP_LOGI(TAG, "Ableton Link enabled");

    // Start the LED tick task (high priority for accurate timing)
    xTaskCreate(tick_task, "tick", 4096, &state, configMAX_PRIORITIES - 1, NULL);

    // Optionally start the print task
    if (PRINT_LINK_STATE) {
        xTaskCreate(print_task, "print", 4096, &state, 1, NULL);
    }

    // Main task can now idle or do other work
    // Link state is managed by the tick_task
}
