#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Requests.h"

#define API_BASE_URL "https://vnxdbpr2-3001.brs.devtunnels.ms"

HTTPClient http;

void sendData(
    bool dhtStatus,
    bool pirStatus,
    bool kyStatus,
    float temperature,
    bool movementDetected,
    int noiseLevel,
    int fanSpeed)
{
    http.begin(String(API_BASE_URL) + "/api/data");

    http.addHeader("Content-Type", "application/json");

    JsonDocument jsonDoc;
    jsonDoc["tempSensorStatus"] = dhtStatus;
    jsonDoc["movSensorStatus"] = pirStatus;
    jsonDoc["noiseSensorStatus"] = kyStatus;
    jsonDoc["temperature"] = temperature;
    jsonDoc["movementDetected"] = movementDetected;
    jsonDoc["noiseLevel"] = noiseLevel;
    jsonDoc["fanSpeed"] = fanSpeed;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = http.POST(jsonString);

    // Verificar o response code

    http.end();
}

void sendPicture()
{
}

int requestFanData()
{
    http.begin(String(API_BASE_URL) + "/api/fan");

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = http.getString();
        JsonDocument jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, payload);

        if (!error)
        {
            bool autoMode = jsonDoc["data"]["autoMode"];
            int fanSpeed = jsonDoc["data"]["fanSpeed"];

            if (autoMode)
                return -1;
            else
                return fanSpeed;
        }
        else
            Serial.println("[REQUEST FAN DATA] Error Parsin Fan Data JSON");
    }
    else
        Serial.println("[REQUEST FAN DATA] Request Failed: " + String(httpCode));

    http.end();

    return -2;
}
