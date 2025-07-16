#include <Arduino.h>

#include <Validations.h>
#include <Connection.h>
#include <Requests.h>
#include <Dht11.h>
#include <Pir.h>
#include <Ky.h>
#include <Fan.h>
#include <MicroServo.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleReadMovement = NULL;
TaskHandle_t xTaskHandleReadNoise = NULL;
TaskHandle_t xTaskHandleSetFanSpeed = NULL;
TaskHandle_t xTaskHandleSwingServo = NULL;
TaskHandle_t xTaskHandleRequestFanData = NULL;
TaskHandle_t xTaskHandleSendData = NULL;

QueueHandle_t xQueueHandleTemperature = NULL;
QueueHandle_t xQueueHandleMovement = NULL;
QueueHandle_t xQueueHandleNoise = NULL;
QueueHandle_t xQueueHandleFanSpeed = NULL;

void vTaskReadTemperature(void *pvParams);
void vTaskReadMovement(void *pvParams);
void vTaskReadNoise(void *pvParams);
void vTaskSetFanSpeed(void *pvParams);
void vTaskSwingServo(void *pvParams);
void vTaskRequestFanData(void *pvParams);
void vTaskSendData(void *pvParams);

void setup()
{
  Serial.begin(115200);

  connectionSetup();
  dhtSetup();
  pirSetup();
  kySetup();
  fanSetup();
  servoSetup();

  xQueueHandleTemperature = xQueueCreate(10, sizeof(float));
  xQueueHandleMovement = xQueueCreate(10, sizeof(bool));
  xQueueHandleNoise = xQueueCreate(10, sizeof(int));
  xQueueHandleFanSpeed = xQueueCreate(10, sizeof(int));

  xTaskCreatePinnedToCore(vTaskReadTemperature, "[TASK 1] Read Temperature", 2048, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskReadMovement, "[TASK 2] Read Movement", 2048, NULL, 1, &xTaskHandleReadMovement, 0);
  xTaskCreatePinnedToCore(vTaskReadNoise, "[TASK 3] Read Noise", 2048, NULL, 1, &xTaskHandleReadNoise, 0);
  xTaskCreatePinnedToCore(vTaskSetFanSpeed, "[TASK 4] Set Fan Speed", 2048, NULL, 1, &xTaskHandleSetFanSpeed, 0);
  xTaskCreatePinnedToCore(vTaskSwingServo, "[TASK 5] Swing Servo", 8192, NULL, 1, &xTaskHandleSwingServo, 0);
  xTaskCreatePinnedToCore(vTaskRequestFanData, "[TASK 6] Request Fan Data", 8192, NULL, 1, &xTaskHandleRequestFanData, 1);
  xTaskCreatePinnedToCore(vTaskSendData, "[TASK 7] Send Data", 8192, NULL, 1, &xTaskHandleSendData, 1);
}

void loop()
{
}

void vTaskReadTemperature(void *pvParams)
{
  while (1)
  {
    float temperature = getTemperature();

    if (validateTemperatureReading(temperature))
    {
      setDhtStatus(true);
      xQueueSend(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(800));
      Serial.println("[TASK 1] Reading Temperature...");
    }
    else
    {
      setDhtStatus(false);
      Serial.println("[TASK 1] Error Reading Temperature!");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskReadMovement(void *pvParams)
{
  while (1)
  {
    bool movementDetected = isMovementDetected();

    if (validateMovementReading(movementDetected))
    {
      setPirStatus(true);
      xQueueSend(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(800));
      Serial.println("[TASK 2] Reading Movement...");
    }
    else
    {
      setPirStatus(false);
      Serial.println("[TASK 2] Error Reading Movement!");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskReadNoise(void *pvParams)
{
  while (1)
  {
    int noiseLevel = getNoiseLevel();

    if (validateNoiseLevelReading(noiseLevel))
    {
      setKyStatus(true);
      xQueueSend(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(800));
      Serial.println("[TASK 3] Reading Noise Level...");
    }
    else
    {
      Serial.println("[TASK 3] Error Reading Noise Level!");
      setKyStatus(false);
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskSetFanSpeed(void *pvParams)
{
  while (1)
  {
    int fanSpeed;

    if (isFanAutoMode())
    {
      float temperature;

      if (xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(800)))
      {
        if (temperature < 30.8)
        {
          fanSpeed = 0;
        }
        else if (temperature >= 30.8 && temperature < 32.0)
        {
          fanSpeed = 120;
        }
        else
          fanSpeed = 255;

        Serial.println("[TASK 4] Automatically Setting Fan Speed...");
      }
      else
        Serial.println("[TASK 4] Temperature Queue is Empty, Fan Speed Was Mantained!");
    }
    else
    {
      fanSpeed = getFanPWM();
      Serial.println("[TASK 4] Manually Setting Fan Speed from API Data...");
    }

    setFanPWM(fanSpeed);
    xQueueSend(xQueueHandleFanSpeed, &fanSpeed, pdMS_TO_TICKS(800));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSwingServo(void *pvParams)
{
  while (1)
  {
    bool movementDetected;

    if (xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(800)))
    {
      if (movementDetected)
      {
        startSwing();
        Serial.println("[TASK 5] Swinging Micro Servo...");
        sendServoEvent();
        vTaskDelay(pdMS_TO_TICKS(5000));
      }
      else
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    else
    {
      Serial.println("[TASK 5] Movement Detection Queue is Empty!");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
}

void vTaskRequestFanData(void *pvParams)
{
  while (1)
  {
    int fanSpeed = requestFanData();

    if (fanSpeed >= 0)
    {
      setFanAutoMode(false);
      setFanPWM(fanSpeed);
      Serial.println("[TASK 6] Fan Set to Manual Mode, Fan PWM Received from API: " + String(fanSpeed));
    }
    else if (fanSpeed == -1)
    {
      setFanAutoMode(true);
      Serial.println("[TASK 6] Fan Set to Auto Mode!");
    }
    else
    {
      Serial.println("[TASK 6] Error Requesting Fan Data from API!");
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}

void vTaskSendData(void *pvParams)
{
  while (1)
  {
    bool dhtStatus = getDhtStatus();
    bool pirStatus = getPirStatus();
    bool kyStatus = getKyStatus();

    float temperature;
    bool movementDetected;
    int noiseLevel;

    bool fanAutoMode = isFanAutoMode();
    int fanSpeed;

    if (!xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)))
    {
      Serial.println("[TASK 7] Temperature Queue is Empty!");
      temperature = 0.0;
    }

    if (!xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000)))
    {
      Serial.println("[TASK 7] Movement Detection Queue is Empty!");
      movementDetected = false;
    }

    if (!xQueueReceive(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(1000)))
    {
      Serial.println("[TASK 7] Noise Level Queue is Empty!");
      noiseLevel = 0;
    }

    if (!xQueueReceive(xQueueHandleFanSpeed, &fanSpeed, pdMS_TO_TICKS(1000)))
    {
      Serial.println("[TASK 7] Fan Speed Queue is Empty!");
      fanSpeed = getFanPWM();
    }

    if (isConnected())
    {
      sendData(dhtStatus, pirStatus, kyStatus, temperature, movementDetected, noiseLevel, fanAutoMode, fanSpeed);
      Serial.println("[TASK 7] Sending Data to API...");
    }
    else
      Serial.println("[TASK 7] WiFi is Not Connected, Data Not Sent!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}