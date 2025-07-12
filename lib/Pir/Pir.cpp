#include <Arduino.h>
#include "Pins.h"

volatile bool pirStatus = false;

void pirSetup()
{
    pinMode(PIR_PIN, INPUT);
    pirStatus = true;
    Serial.println("[PIR HC-SR501 SETUP] Sensor Setup Sucessful!");
}

bool getPirStatus(){
    return pirStatus;
}

void setPirStatus(bool newStatus){
    pirStatus = newStatus;
}

bool isMovementDetected()
{
    return digitalRead(PIR_PIN);
}