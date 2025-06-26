#include <Arduino.h>
#include "Pins.h"

#define FAN_FREQ 25000
#define RESOLUTION 8

volatile int fanPWM = 60;
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

void IRAM_ATTR setFanPWM(float temperature)
{
    if (autoMode)
    {
        if (temperature < 32.0)
        {
            fanPWM = 0;
        }
        else if (temperature >= 32.0 && temperature < 34.90)
        {
            fanPWM = 120;
        }
        else
            fanPWM = 255;
    }
    else
        fanPWM; 

    ledcWrite(0, 255 - fanPWM);
}

bool isAutoMode()
{
    return autoMode;
}

void setAutoMode(bool newMode)
{
    autoMode = newMode;
}