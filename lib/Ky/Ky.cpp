#include <Arduino.h>
#include "Pins.h"

volatile bool kyStatus = false;

void kySetup()
{
    pinMode(KY_PIN, INPUT);
    kyStatus = true;
    Serial.println("[KY-037 SETUP] Sensor Setup Sucessful!");
}

bool getKyStatus(){
    return kyStatus;
}

void setKyStatus(bool newStatus){
    kyStatus = newStatus;
}

int getNoiseLevel()
{
    return analogRead(KY_PIN);
}