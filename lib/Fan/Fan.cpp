#include <Arduino.h>
#include "Pins.h"
#include "Fan.h"

#define FAN_FREQ 25000
#define RESOLUTION 8

volatile int fanPWM = 0;
volatile bool autoMode = true;

void fanSetup()
{
    ledcAttachPin(FAN_PWM_PIN, 0);
    ledcSetup(0, FAN_FREQ, RESOLUTION);
}

int getFanPWM()
{
    return fanPWM;
}

void setFanPWM(int newFanPWM)
{
    fanPWM = newFanPWM;
    ledcWrite(0, 255 - fanPWM);
}

bool isFanAutoMode()
{
    return autoMode;
}

void setFanAutoMode(bool newMode)
{
    autoMode = newMode;
}