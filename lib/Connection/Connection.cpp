#include <Arduino.h>
#include <SPI.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define SSID "SSID"
#define PASSWORD "PASSWORD"

void connectionSetup()
{
    WiFi.begin(SSID, PASSWORD);

    Serial.println("Connecting...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("Connecting...");
    }

    Serial.println("Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

void sendData()
{
}

void sendPicture()
{
}

int getFanData()
{
    return 120;
}