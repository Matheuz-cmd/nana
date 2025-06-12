#include <Arduino.h>

#include <Dht.h>
#include <Pir.h>
#include <Fan.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleReadMovement = NULL;
// TaskHandle_t xTaskHandleSetFanSpeed = NULL;
TaskHandle_t xTaskHandlePrintStatus = NULL;

QueueHandle_t xQueueHandleTemperature = NULL;
QueueHandle_t xQueueHandleMovement = NULL;
QueueHandle_t xQueueHandleFanSpeed = NULL;

void vTaskReadTemperature(void *pvParams);
void vTaskReadMovement(void *pvParams);
// void vTaskSetFanSpeed(void *pvParams);
void vTaskPrintStatus(void *pvParams);

void setup()
{
  Serial.begin(115200);

  dhtSetup();
  pirSetup();
  // fanSetup();

  xQueueHandleTemperature = xQueueCreate(10, sizeof(float));
  xQueueHandleMovement = xQueueCreate(10, sizeof(bool));
  // xQueueHandleFanSpeed = xQueueCreate(10, sizeof(float));

  xTaskCreatePinnedToCore(vTaskReadTemperature, "Read Temperature", 4096, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskReadMovement, "Read Movement", 4096, NULL, 1, &xTaskHandleReadMovement, 0);
  // xTaskCreatePinnedToCore(vTaskSetFanSpeed, "Set Fan Speed", 4096, NULL, 1, &xTaskHandleSetFanSpeed, 0);
  xTaskCreatePinnedToCore(vTaskPrintStatus, "Printing Status", 4096, NULL, 1, &xTaskHandlePrintStatus, 0);
}

void loop()
{
}

void vTaskReadTemperature(void *pvParams)
{
  while (1)
  {
    float temperature = getTemperature();

    Serial.print(temperature);

    if (isnan(temperature))
    {
      Serial.println("TASK 1: Error reading temperature!");
    }
    else
    {
      Serial.println("TASK 1: Reading temperature...");
      xQueueSend(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskReadMovement(void *pvParams)
{
  while (1)
  {
    bool movementDetected = isMovementDetected();

    if (isnan(movementDetected))
    {
      Serial.println("TASK 2: Error detecting movement!");
    }
    else
    {
      Serial.println("TASK 2: Detecting movement...");
      xQueueSend(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(200));
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// void vTaskSetFanSpeed(void *pvParams)
// {
//   while (1)
//   {
//     float temperature;
//     int newFanPwm;

//     xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000));

//     if (isnan(temperature))
//     {
//       Serial.println("TASK 3: Error Setting Fan Speed, Temperature Not Read!");
//     }
//     else
//     {
//       if (temperature < 32.0)
//       {
//         newFanPwm = 0;
//       }
//       else if (32.0 <= temperature < 34.0)
//       {
//         newFanPwm = 80;
//       }
//       else
//       {
//         newFanPwm = 255;
//       }

//       setFanSpeed(newFanPwm);
//       xQueueSend(xQueueHandleFanSpeed, &newFanPwm, pdMS_TO_TICKS(200));
//     }

//     vTaskDelay(pdMS_TO_TICKS(1000));
//   }
// }

void vTaskPrintStatus(void *pvParams)
{
  float temperature;
  bool movementDetected;
  // int fanPwm;

  while (1)
  {
    if (
        xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000))
        )
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print("°C | Movement Detection: ");
      Serial.print(movementDetected ? "Detected" : "No Movement");
      // Serial.print(" | Fan Speed: ");
      // Serial.print((fanPwm / 255) * 100);
      // Serial.println("%");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}