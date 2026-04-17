#include "mavesp8266.h"
#include "board_pins.h"

#ifndef LED_MANAGER_H
#define LED_MANAGER_H


#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
#include <Adafruit_NeoPixel.h>

#define GPIO_RGB_LED   8 // Onboard RGB LED is usually on Pin 8 for ESP32-C3 SuperMini
#define NUMPIXELS      1 // Only one LED on the ESP32-C3 SuperMini  
extern Adafruit_NeoPixel pixels;
#endif

class LEDManager
{
public:
    enum Led
    {
        wifi = KAHUNA_PIN_LED_WIFI,
        air = KAHUNA_PIN_LED_AIR,
        gcs = KAHUNA_PIN_LED_GCS
    };
    enum LedStatus
    {
        off,
        on,
        blink,
        doubleBlink
    };
    void init();
    void setLED(Led selectedLed, LedStatus status);
    void blinkLED();
    void doubleBlinkLED();

private:
    unsigned long _timeNextBlink = 0;       // Time at which the next change in the status light is due
    unsigned long _timeNextDoubleBlink = 0; // Time at which the next double blink is due
    bool _ledsToBlink = false;
    LedStatus _gcsLedStatus = off;
    LedStatus _wifiLedStatus = off;
    LedStatus _airLedStatus = off;
    int _gcsValue = LOW;
    int _wifiValue = LOW;
    int _airValue = LOW;
    int _cycleTime = 600;
    // bool _doubleBlinkFlag = false;
    int _doubleBlinkCount = 0;
};
#endif