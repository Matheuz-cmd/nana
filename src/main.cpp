#include <Connection.h>
#include <Dht11.h>
#include <Pir.h>
#include <Ky.h>
#include <Fan.h>
// #include <Cam.h>
// #include <MicroServo.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleReadMovement = NULL;
TaskHandle_t xTaskHandleReadNoise = NULL;
TaskHandle_t xTaskHandleSetFanSpeed = NULL;
TaskHandle_t xTaskHandleTakePicture = NULL;
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
void vTaskTakePicture(void *pvParams);
void vTaskSwingServo(void *pvParams);
void vTaskRequestFanData(void *pvParams);
void vTaskSendData(void *pvParams);

void setup()
{
  Serial.begin(115200);

  // connectionSetup();
  dhtSetup();
  pirSetup();
  kySetup();
  fanSetup();
  // camSetup();
  // servoSetup();

  xQueueHandleTemperature = xQueueCreate(10, sizeof(float));
  xQueueHandleMovement = xQueueCreate(10, sizeof(bool));
  xQueueHandleNoise = xQueueCreate(10, sizeof(int));
  xQueueHandleFanSpeed = xQueueCreate(10, sizeof(int));

  xTaskCreatePinnedToCore(vTaskReadTemperature, "[TASK 1] Read Temperature", 4096, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskReadMovement, "[TASK 2] Read Movement", 4096, NULL, 1, &xTaskHandleReadMovement, 0);
  xTaskCreatePinnedToCore(vTaskReadNoise, "[TASK 3] Read Noise", 4096, NULL, 1, &xTaskHandleReadNoise, 0);
  xTaskCreatePinnedToCore(vTaskSetFanSpeed, "[TASK 4] Set Fan Speed", 4096, NULL, 1, &xTaskHandleSetFanSpeed, 0);
  xTaskCreatePinnedToCore(vTaskTakePicture, "[TASK 5] Take Picture", 4096, NULL, 1, &xTaskHandleTakePicture, 0);
  xTaskCreatePinnedToCore(vTaskSwingServo, "[TASK 6] Swing Servo", 4096, NULL, 1, &xTaskHandleSwingServo, 0);
  //xTaskCreatePinnedToCore(vTaskRequestFanData, "[TASK 7] Request Fan Data From API", 4096, NULL, 1, &xTaskHandleRequestFanData, 1);
  xTaskCreatePinnedToCore(vTaskSendData, "[TASK 8] Send Data", 4096, NULL, 1, &xTaskHandleSendData, 1);
}

void loop()
{
}

void vTaskReadTemperature(void *pvParams)
{
  while (1)
  {
    float temperature = getTemperature();

    if (!isnan(temperature))
    {
      Serial.println("[TASK 1] Reading Temperature...");
      xQueueSend(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(800));
    }
    else
      Serial.println("[TASK 1] Error Reading Temperature!");

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskReadMovement(void *pvParams)
{
  while (1)
  {
    bool movementDetected = isMovementDetected();

    if (movementDetected == true || movementDetected == false)
    {
      Serial.println("[TASK 2] Reading Movement...");
      xQueueSend(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(800));
    }
    else
    {
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

    if (noiseLevel >= 0 && noiseLevel <= 4095)
    {
      Serial.println("[TASK 3] Reading Noise Level...");
      xQueueSend(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(800));
    }
    else
    {
      Serial.println("[TASK 3] Error Reading Noise Level!");
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskSetFanSpeed(void *pvParams)
{
  while (1)
  {
    int fanSpeed;
    float temperature;

    if (xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(800)) == pdTRUE)
    {
      if (isAutoMode())
        Serial.println("[TASK 4] Automatically Setting Fan Speed...");
      else
        Serial.println("[TASK 4] Manually Setting Fan Speed from API Data...");

      setFanPWM(temperature);
    }
    else
      Serial.println("[TASK 4] Temperature Queue is Empty!");

    fanSpeed = getFanPWM();
    xQueueSend(xQueueHandleFanSpeed, &fanSpeed, pdMS_TO_TICKS(800));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskTakePicture(void *pvParams)
{
  while (1)
  {
    bool movementDetected;

    if (xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(800)) == pdTRUE)
    {
      if (movementDetected == true)
      {
        // type pic = takePicture();
        // sendPicture(pic);
        Serial.println("[TASK 5] Taking Picture...");
      }
    }
    else
      Serial.println("[TASK 5] Movement Detection Queue is Empty!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSwingServo(void *pvParams)
{
  while (1)
  {
    bool movementDetected;

    if (xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(800)) == pdTRUE)
    {
      if (movementDetected == true)
      {
        // startSwing();
        Serial.println("[TASK 6] Swinging Micro Servo...");
      }
    }
    else
      Serial.println("[TASK 6] Movement Detection Queue is Empty!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskRequestFanData(void *pvParams)
{
  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void vTaskSendData(void *pvParams)
{
  float temperature;
  bool movementDetected;
  int noiseLevel;
  int fanSpeed;

  while (1)
  {
    if (
        xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleFanSpeed, &fanSpeed, pdMS_TO_TICKS(1000)))
    {
      Serial.println("\nTemperature (°C): ");
      Serial.println(temperature);
      Serial.println("\nMovement Detection: ");
      Serial.println(movementDetected ? "Detected" : "No Movement");
      Serial.println("\nNoise Level: ");
      Serial.println(noiseLevel);
      Serial.println("\nFan Speed (PWM): ");
      Serial.println(fanSpeed);
      Serial.println("\n");
    }

    //senData()

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}