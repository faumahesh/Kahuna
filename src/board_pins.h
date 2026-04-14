#ifndef KAHUNA_BOARD_PINS_H
#define KAHUNA_BOARD_PINS_H

/*
 * Default GPIO assignments per architecture. Override with -DKAHUNA_PIN_* in platformio.ini
 * for your PCB (ESP8266 esp07s vs ESP32-C3/C6 devkits differ).
 */

#if defined(ARDUINO_ARCH_ESP8266)

#ifndef KAHUNA_PIN_LED_WIFI
#define KAHUNA_PIN_LED_WIFI 4
#endif
#ifndef KAHUNA_PIN_LED_AIR
#define KAHUNA_PIN_LED_AIR 12
#endif
#ifndef KAHUNA_PIN_LED_GCS
#define KAHUNA_PIN_LED_GCS 5
#endif
#ifndef KAHUNA_PIN_FACTORY_RESET
#define KAHUNA_PIN_FACTORY_RESET 2
#endif

#elif defined(ARDUINO_ARCH_ESP32)

#ifndef KAHUNA_PIN_LED_WIFI
#define KAHUNA_PIN_LED_WIFI 8
#endif
#ifndef KAHUNA_PIN_LED_AIR
#define KAHUNA_PIN_LED_AIR 6
#endif
#ifndef KAHUNA_PIN_LED_GCS
#define KAHUNA_PIN_LED_GCS 7
#endif
#ifndef KAHUNA_PIN_FACTORY_RESET
#define KAHUNA_PIN_FACTORY_RESET 2
#endif
#ifndef KAHUNA_VEHICLE_RX_PIN
#define KAHUNA_VEHICLE_RX_PIN 5
#endif
#ifndef KAHUNA_VEHICLE_TX_PIN
#define KAHUNA_VEHICLE_TX_PIN 4
#endif

#else
#error "Unsupported architecture (need ESP8266 or ESP32 Arduino)"
#endif

#endif
