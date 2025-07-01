#include <Arduino.h>
#include <DHT.h>
#include "Pins.h"

#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

volatile bool dhtStatus = false;

void dhtSetup()
{
    dht.begin();
    dhtStatus = true;
    Serial.println("[DHT11 SETUP] Sensor Setup Sucessful!");
}

bool getDhtStatus(){
    return dhtStatus;
}
 
void setDhtStatus(bool newStatus){
    dhtStatus = newStatus;
}

float getTemperature()
{
    return dht.readTemperature();
}
