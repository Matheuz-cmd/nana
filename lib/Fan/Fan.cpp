#include <Arduino.h>
#include "Pins.h"
#include "Fan.h"

#define FAN_FREQ 25000
#define RESOLUTION 8
#define FAN_PWM_CHANNEL 0

volatile int fanPWM = 0;
volatile bool autoMode = true;

void fanSetup()
{
    ledcSetup(FAN_PWM_CHANNEL, FAN_FREQ, RESOLUTION);
    ledcAttachPin(FAN_PWM_PIN, FAN_PWM_CHANNEL);
    Serial.println("[FAN SETUP] Fan Setup Sucessful!");
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