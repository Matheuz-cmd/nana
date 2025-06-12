#include "Pins.h"
#include <DHT.h>
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

void dhtSetup()
{
    dht.begin();
}

float getTemperature()
{
    Serial.print(dht.readTemperature());
    return dht.readTemperature();
}
