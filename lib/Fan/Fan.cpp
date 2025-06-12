#include <Arduino.h>
#include "Pins.h"

volatile int fanPwm = 0;

void fanSetup()
{
    // PWM setup: canal 0, frequência 25 kHz, 8 bits
    ledcSetup(0, 25000, 8);
    ledcAttachPin(FAN_PWM_PIN, 0); // PWM no fio azul da fan
}

void setFanSpeed(int newFanPwm)
{
    fanPwm = newFanPwm;
    ledcWrite(0, 255 - fanPwm);
}
