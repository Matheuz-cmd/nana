#include <Arduino.h>
#include "Pins.h"

#define FAN_FREQ 25000
#define RESOLUTION 8

volatile int fanPwm = 120;
volatile bool autoMode = true; 

void fanSetup()
{
    ledcAttachPin(FAN_PWM_PIN, 0); 
    ledcSetup(0, FAN_FREQ, RESOLUTION);
}

int getFanSpeed()
{
    return fanPwm;
}

void setFanSpeed(int newFanPwm)
{
    fanPwm = newFanPwm;
    ledcWrite(0, 255 - fanPwm);
}

bool isAutoMode()
{
    return autoMode;
}

void setAutoMode(bool newAutoMode)
{
    autoMode = newAutoMode;
}