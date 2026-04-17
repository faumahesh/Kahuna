#include "led_manager.h"

#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
Adafruit_NeoPixel pixels(NUMPIXELS, GPIO_RGB_LED, NEO_GRB + NEO_KHZ800);

static bool ledChannelActive(LEDManager::LedStatus status, int value)
{
    if (status == LEDManager::off)
    {
        return false;
    }
    return value != 0;
}

static void showRgbStatus(LEDManager::LedStatus gcsStatus, int gcsValue,
                          LEDManager::LedStatus wifiStatus, int wifiValue,
                          LEDManager::LedStatus airStatus, int airValue)
{
    const bool wifiActive = ledChannelActive(wifiStatus, wifiValue);
    const bool gcsActive = ledChannelActive(gcsStatus, gcsValue);
    const bool airActive = ledChannelActive(airStatus, airValue);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;

    // Priority mode on single RGB LED: WiFi > GCS > Air.
    if (wifiActive)
    {
        green = 150;
    }
    else if (gcsActive)
    {
        red = 150;
    }
    else if (airActive)
    {
        blue = 150;
    }

    pixels.setPixelColor(0, pixels.Color(red, green, blue));
    pixels.show();
}
#endif

void LEDManager::init()
{
#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
    pixels.begin();
    pixels.clear();
    pixels.show();
#endif
}

void LEDManager::setLED(Led selectedLed, LedStatus ledStatus)
{
    switch (selectedLed)
    {
    case gcs:
        if (_gcsLedStatus != ledStatus)
        {
            _gcsLedStatus = ledStatus;
            switch (_gcsLedStatus)
            {
            case off:
                _gcsValue = LOW;
                break;
            case on:
                _gcsValue = HIGH;
                break;
            default:
                break;
            }
        }
        break;
    case wifi:
        if (_wifiLedStatus != ledStatus)
        {
            _wifiLedStatus = ledStatus;
            switch (_wifiLedStatus)
            {
            case off:
                _wifiValue = LOW;
                break;
            case on:
                _wifiValue = HIGH;
                break;
            case doubleBlink:
                _wifiValue = LOW;
            default:
                break;
            }
        }
        break;
    case air:
        if (_airLedStatus != ledStatus)
        {
            _airLedStatus = ledStatus;
            switch (_airLedStatus)
            {
            case off:
                _airValue = LOW;
                break;
            case on:
                _airValue = HIGH;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
    showRgbStatus(_gcsLedStatus, _gcsValue, _wifiLedStatus, _wifiValue, _airLedStatus, _airValue);
#else
    digitalWrite(gcs, _gcsValue);
    digitalWrite(wifi, _wifiValue);
    digitalWrite(air, _airValue);
#endif
}
void LEDManager::blinkLED()
{
    if (millis() >= _timeNextBlink)
    {
        _timeNextBlink = millis() + _cycleTime;
        if (_gcsLedStatus == blink)
        {
            _gcsValue = !_gcsValue;
        }
        if (_wifiLedStatus == blink)
        {
            _wifiValue = !_wifiValue;
        }
        if (_airLedStatus == blink)
        {
            _airValue = !_airValue;
        }

#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
        showRgbStatus(_gcsLedStatus, _gcsValue, _wifiLedStatus, _wifiValue, _airLedStatus, _airValue);
#else
        digitalWrite(gcs, _gcsValue);
        digitalWrite(wifi, _wifiValue);
        digitalWrite(air, _airValue);
#endif
    }
}

// double blink on for 200, off for 200, on for 200, off for 600

void LEDManager::doubleBlinkLED()
{
    if (millis() >= _timeNextDoubleBlink && _doubleBlinkCount < 3)
    {
        _doubleBlinkCount++;
        _timeNextDoubleBlink = millis() + (_cycleTime / 3);
        if (_wifiLedStatus == doubleBlink)
        {
            _wifiValue = !_wifiValue;
        }
    }
    else if (millis() >= _timeNextDoubleBlink)
    {
        _doubleBlinkCount = 0;
        _timeNextDoubleBlink = millis() + _cycleTime;
        if (_wifiLedStatus == doubleBlink)
        {
            _wifiValue = !_wifiValue;
        }
    }

#if defined(KAHUNA_BOARD_ESP32C3_SUPERMINI)
    showRgbStatus(_gcsLedStatus, _gcsValue, _wifiLedStatus, _wifiValue, _airLedStatus, _airValue);
#else
    digitalWrite(wifi, _wifiValue);
#endif
}