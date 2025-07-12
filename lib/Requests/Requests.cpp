#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Requests.h"
#include "Secrets.h"

WiFiClientSecure *dataClient = new WiFiClientSecure;
WiFiClientSecure *servoClient = new WiFiClientSecure;
WiFiClientSecure *fanClient = new WiFiClientSecure;
HTTPClient httpData;
HTTPClient httpServo;
HTTPClient httpFan;

void sendData(
    bool dhtStatus,
    bool pirStatus,
    bool kyStatus,
    float temperature,
    bool movementDetected,
    int noiseLevel,
    bool fanAutoMode,
    int fanSpeed)
{
    dataClient->setCACert(CERT);
    httpData.begin(*dataClient, String(API_URL) + "/api/data");

    httpData.addHeader("Content-Type", "application/json");

    JsonDocument jsonDoc;
    jsonDoc["tempSensorStatus"] = dhtStatus;
    jsonDoc["movSensorStatus"] = pirStatus;
    jsonDoc["noiseSensorStatus"] = kyStatus;
    jsonDoc["temperature"] = temperature;
    jsonDoc["movementDetected"] = movementDetected;
    jsonDoc["noiseLevel"] = noiseLevel;
    jsonDoc["fanAutoMode"] = fanAutoMode;
    jsonDoc["fanSpeed"] = fanSpeed;

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = httpData.POST(jsonString);

    httpData.end();
}

void sendServoEvent()
{
    servoClient->setCACert(CERT);
    httpServo.begin(*servoClient, String(API_URL) + "/api/event");

    httpServo.addHeader("Content-Type", "application/json");

    JsonDocument jsonDoc;
    jsonDoc["name"] = "servo-swing";

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    Serial.println(jsonString);

    int httpResponseCode = httpServo.POST(jsonString);

    httpServo.end();
}

int requestFanData()
{
    fanClient->setCACert(CERT);
    httpFan.begin(*fanClient, String(API_URL) + "/api/fan");

    int httpCode = httpFan.GET();

    if (httpCode == HTTP_CODE_OK)
    {
        String payload = httpFan.getString();
        JsonDocument jsonDoc;
        DeserializationError error = deserializeJson(jsonDoc, payload);

        if (!error)
        {
            bool autoMode = jsonDoc["data"]["fanAutoMode"];
            int fanSpeed = jsonDoc["data"]["fanSpeed"];

            if (autoMode)
                return -1;
            else
                return fanSpeed;
        }
        else
            Serial.println("[REQUEST FAN DATA] Error Parsing Fan Data JSON");
    }
    else
        Serial.println("[REQUEST FAN DATA] Request Failed: " + String(httpCode));

    httpFan.end();

    return -2;
}
