#pragma once
#include <Arduino.h>

namespace PinData
{
    // Set HIGH or LOW to match the trigger of your relays
    const int Trigger = HIGH;

    // Pins in order
#if defined(ESP32)
    const int IDs[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
#elif defined(ESP8266)
    // For my D1 Mini clone. Change as is appropriate for your board
    const int IDs[8] = { D1, D2, D3, D4, D5, D6, D7, D8 };
#endif
    const int Count = sizeof(IDs) / sizeof(IDs[0]);
}
