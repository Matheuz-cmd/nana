#include <Arduino.h>
#include "Pins.h"

void kySetup()
{
    pinMode(KY_PIN, INPUT);
}

int getNoiseLevel()
{
    return analogRead(KY_PIN);
}