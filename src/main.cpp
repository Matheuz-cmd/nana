#include <Arduino.h>

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
TaskHandle_t xTaskHandlePrintStatus = NULL; // change to senData task further 

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
void vTaskPrintStatus(void *pvParams);

void setup()
{
  Serial.begin(115200);

  dhtSetup();
  pirSetup();
  // kySetup();
  fanSetup();
  // camSetup();
  // servoSetup();

  xQueueHandleTemperature = xQueueCreate(10, sizeof(float));
  xQueueHandleMovement = xQueueCreate(10, sizeof(bool));
  xQueueHandleNoise = xQueueCreate(10, sizeof(int));
  xQueueHandleFanSpeed = xQueueCreate(10, sizeof(int));

  xTaskCreatePinnedToCore(vTaskReadTemperature, "[TASK 1] Read Temperature", 4096, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskReadMovement, "[TASK 2] Read Movement", 4096, NULL, 1, &xTaskHandleReadMovement, 0);
  // xTaskCreatePinnedToCore(vTaskReadNoise, "[TASK 3] Read Noise", 4096, NULL, 1, &xTaskHandleReadNoise, 0);
  xTaskCreatePinnedToCore(vTaskSetFanSpeed, "[TASK 4] Set Fan Speed", 4096, NULL, 1, &xTaskHandleSetFanSpeed, 0);
  // xTaskCreatePinnedToCore(vTaskTakePicture, "[TASK 5] Take PiCture", 4096, NULL, 1, &xTaskHandleTakePicture, 0);
  // xTaskCreatePinnedToCore(vTaskSwingServo, "[TASK 6] Swing Servo", 4096, NULL, 1, &xTaskHandleSwingServo, 0);
  xTaskCreatePinnedToCore(vTaskPrintStatus, "[TASK 7] Printing Status", 4096, NULL, 1, &xTaskHandlePrintStatus, 0);
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
      Serial.println("TASK 1: Reading Temperature...");
      xQueueSend(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000));
    }
    else
      Serial.println("TASK 1: Error reading temperature!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskReadMovement(void *pvParams)
{
  while (1)
  {
    bool movementDetected = isMovementDetected();

    if (movementDetected == true || movementDetected == false)
    {
      Serial.println("TASK 2: Reading Movement...");
      xQueueSend(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(200));
    }
    else
    {
      Serial.println("TASK 2: Error Reading Movement!");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskReadNoise(void *pvParams)
{
  while (1)
  {
    int noiseLevel = getNoiseLevel();

    if (noiseLevel >= 0 && noiseLevel <= 4095)
    {
      Serial.println("TASK 3: Reading Noise Level...");
      xQueueSend(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(200));
    }
    else
    {
      Serial.println("TASK 3: Error Reading Noise Level!");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSetFanSpeed(void *pvParams)
{
  while (1)
  {
    int fanPwm; 
    bool autoMode = isAutoMode();

    if (autoMode)
    {
      float temperature;

      if (xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) == pdTRUE)
      {
        if (!isnan(temperature))
        {
          if (temperature < 32.0)
            fanPwm = 0;
          else if (temperature >= 32.0 && temperature < 34.90)
            fanPwm = 10;
          else
            fanPwm = 255;
            
          Serial.println("TASK 4: Automatically Setting Fan Speed...");
        }
      }
      else
        Serial.println("TASK 4: Error Reading Temperature Values From Queue!");
    }
    else
    {
      fanPwm = getFanSpeed();
      Serial.println("TASK 4: Manually Setting Fan Speed...");
    }

    setFanSpeed(fanPwm);
    xQueueSend(xQueueHandleFanSpeed, &fanPwm, pdMS_TO_TICKS(200));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskTakePicture(void *pvParams)
{
  while (1)
  {
    bool movementDetected;

    if (xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
      if (movementDetected == true){
        // Function to Take Picture in the Cam.cpp lib;
        Serial.println("TASK 5: Taking Picture...");
      }
    }
    else
      Serial.println("TASK 5: Error Reading Movement Detection Values From Queue!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSwingServo(void *pvParams)
{
  while (1)
  {
    int noiseLevel;

    if (xQueueReceive(xQueueHandleNoise, &noiseLevel, pdMS_TO_TICKS(1000)) == pdTRUE)
    {
      if (noiseLevel >= 4000){
        // startSwing();
        Serial.println("TASK 6: Swinging Micro Servo...");
      }
    }
    else
      Serial.println("TASK 6: Error Reading Noise Levels From Queue!");

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskPrintStatus(void *pvParams)
{
  float temperature;
  bool movementDetected;
  int noiseLevel;
  int fanPwm;

  while (1)
  {
    if (
        xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleFanSpeed, &fanPwm, pdMS_TO_TICKS(1000)))
    {
      Serial.println("\nTemperature (°C): ");
      Serial.print(temperature);
      Serial.println("\nNoise Level: ");
      Serial.print(noiseLevel);
      Serial.println("\nMovement Detection: ");
      Serial.print(movementDetected ? "Detected" : "No Movement");
      Serial.println("\nFan PWM: ");
      Serial.print(fanPwm);
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}