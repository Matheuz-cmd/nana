#include <WiFiClientSecure.h>

#define SSID "SSID"
#define PASSWORD "PASSWORD"

void connectionSetup()
{
    WiFi.begin(SSID, PASSWORD);

    Serial.println("[WIFI SETUP] Connecting...");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.println("[WIFI SETUP] Connecting...");
    }

    Serial.print("[WIFI SETUP] Connected to WiFi network with IP Address: ");
    Serial.println(WiFi.localIP());
}

bool isConnected()
{
    return WiFi.status();
}