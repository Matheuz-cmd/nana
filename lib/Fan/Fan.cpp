#include <Arduino.h>
#include "Pins.h"

volatile int fanPwm = 0;
volatile bool fanAutoMode = true; // NOVO: indica se está no modo automático

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

// NOVO: permite ativar ou desativar o modo automático
void setFanAutoMode(bool enabled)
{
    fanAutoMode = enabled;
}

// NOVO: permite consultar o modo atual
bool isFanAutoMode()
{
    return fanAutoMode;
}
