# Ableton Link Basic Example

A minimal example demonstrating Ableton Link synchronization on ESP32.

Based on the [Ableton Link ESP32 example](https://github.com/Ableton/link/tree/master/examples/esp32) and [abl_link C wrapper example](https://github.com/Ableton/link/tree/master/extensions/abl_link/examples/link_hut).

## What it does

- Connects to WiFi
- Enables Ableton Link with an initial tempo of 120 BPM
- Flashes the built-in LED on each beat
- Prints Link state (peers, tempo, beats, phase) to the serial console

## Hardware

By default, the example uses `GPIO_NUM_2` for the LED, which is the built-in LED on many ESP32 development boards. Modify `LED_GPIO` in `main.c` if your board uses a different pin.

## Building and Running

1. Configure WiFi credentials:

   ```bash
   idf.py menuconfig
   ```

   Navigate to **Example Connection Configuration** and set your WiFi SSID and password.

2. Build and flash:

   ```bash
   idf.py build
   idf.py -p /dev/ttyUSB0 flash monitor
   ```

   Replace `/dev/ttyUSB0` with the serial port your ESP32 is connected to.

## Testing

1. Run a Link-enabled application on your computer or phone (e.g., Ableton Live, or the [LinkHut test app](https://github.com/Ableton/link))
2. Ensure both devices are on the same network
3. The ESP32 should automatically discover and sync with other Link peers
4. The LED will flash in sync with the beat
5. Changes in tempo or phase from other peers will be reflected on the ESP32 and printed to the console