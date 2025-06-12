#include <Arduino.h>
#include "Pins.h"

void pirSetup()
{
    pinMode(PIR_PIN, INPUT);
}

bool isMovementDetected()
{
    return digitalRead(PIR_PIN);
}