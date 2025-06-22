#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include <Dht.h>
#include <Pir.h>
#include <Fan.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

// Struct para comandos vindos da API
typedef struct {
  bool autoMode;
  int manualPwm;
} FanApiCommand;

// Handles de tarefas
TaskHandle_t xTaskHandleReadTemperature = NULL;
TaskHandle_t xTaskHandleReadMovement = NULL;
TaskHandle_t xTaskHandleSetFanSpeed = NULL;
TaskHandle_t xTaskHandlePrintStatus = NULL;
TaskHandle_t xTaskHandleReceiveApi = NULL;

// Queues
QueueHandle_t xQueueHandleTemperature = NULL;
QueueHandle_t xQueueHandleMovement = NULL;
QueueHandle_t xQueueHandleFanSpeed = NULL;
QueueHandle_t xQueueHandleFanApiCommand = NULL;

// Declaração das tasks
void vTaskReadTemperature(void *pvParams);
void vTaskReadMovement(void *pvParams);
void vTaskSetFanSpeed(void *pvParams);
void vTaskReceiveApiCommands(void *pvParams);
void vTaskPrintStatus(void *pvParams);

void setup()
{
  Serial.begin(115200);

  // Conexão Wi-Fi
  WiFi.begin("SEU_WIFI", "SUA_SENHA");
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado!");

  // Inicialização de sensores
  dhtSetup();
  pirSetup();
  fanSetup();

  // Criação das filas
  xQueueHandleTemperature = xQueueCreate(10, sizeof(float));
  xQueueHandleMovement = xQueueCreate(10, sizeof(bool));
  xQueueHandleFanSpeed = xQueueCreate(10, sizeof(int));
  xQueueHandleFanApiCommand = xQueueCreate(5, sizeof(FanApiCommand));

  // Criação das tasks
  xTaskCreatePinnedToCore(vTaskReadTemperature, "Read Temperature", 4096, NULL, 1, &xTaskHandleReadTemperature, 0);
  xTaskCreatePinnedToCore(vTaskReadMovement, "Read Movement", 4096, NULL, 1, &xTaskHandleReadMovement, 0);
  xTaskCreatePinnedToCore(vTaskSetFanSpeed, "Set Fan Speed", 4096, NULL, 1, &xTaskHandleSetFanSpeed, 0);
  xTaskCreatePinnedToCore(vTaskReceiveApiCommands, "Receive API", 4096, NULL, 1, &xTaskHandleReceiveApi, 0);
  xTaskCreatePinnedToCore(vTaskPrintStatus, "Printing Status", 4096, NULL, 1, &xTaskHandlePrintStatus, 0);
}

void loop()
{
  // Nada aqui, FreeRTOS cuida de tudo nas tasks
}

void vTaskReadTemperature(void *pvParams)
{
  while (1)
  {
    float temperature = getTemperature();

    if (isnan(temperature))
    {
      Serial.println("TASK 1: Erro ao ler temperatura!");
    }
    else
    {
      Serial.println("TASK 1: Temperatura lida com sucesso.");
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

    Serial.println("TASK 2: Detectando movimento...");
    xQueueSend(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(200));

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskSetFanSpeed(void *pvParams)
{
  FanApiCommand cmd;
  int manualPwm = 0;

  while (1)
  {
    // Recebe comandos da API
    if (xQueueReceive(xQueueHandleFanApiCommand, &cmd, 0) == pdTRUE)
    {
      setFanAutoMode(cmd.autoMode);
      if (!cmd.autoMode)
      {
        manualPwm = cmd.manualPwm;
        xQueueSend(xQueueHandleFanSpeed, &manualPwm, pdMS_TO_TICKS(100));
      }
    }

    if (isFanAutoMode())
    {
      float temperature;
      if (xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) == pdTRUE)
      {
        int newPwm = 0;

        if (!isnan(temperature))
        {
          if (temperature < 32.0)
            newPwm = 0;
          else if (temperature >= 32.0 && temperature < 34.0)
            newPwm = 80;
          else
            newPwm = 255;

          setFanSpeed(newPwm);
          xQueueSend(xQueueHandleFanSpeed, &newPwm, pdMS_TO_TICKS(200));
        }
      }
    }
    else
    {
      if (xQueueReceive(xQueueHandleFanSpeed, &manualPwm, pdMS_TO_TICKS(500)) == pdTRUE)
      {
        setFanSpeed(manualPwm);
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void vTaskReceiveApiCommands(void *pvParams)
{
  FanApiCommand cmd;

  while (1)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      HTTPClient http;
      http.begin("http://meuservidor.com/api/fan"); // Troque pela URL real da sua API
      int httpResponseCode = http.GET();

      if (httpResponseCode == 200)
      {
        String payload = http.getString();
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (!error)
        {
          cmd.autoMode = doc["auto"];
          cmd.manualPwm = constrain((int)doc["pwm"], 0, 255);

          Serial.printf("API: Modo automático %s | PWM manual: %d\n",
                        cmd.autoMode ? "ativado" : "desativado", cmd.manualPwm);

          xQueueSend(xQueueHandleFanApiCommand, &cmd, pdMS_TO_TICKS(100));
        }
        else
        {
          Serial.println("Erro ao interpretar JSON da API");
        }
      }
      else
      {
        Serial.printf("Erro HTTP da API: %d\n", httpResponseCode);
      }

      http.end();
    }
    else
    {
      Serial.println("WiFi desconectado, não foi possível acessar a API.");
    }

    vTaskDelay(pdMS_TO_TICKS(10000)); // espera 10 segundos
  }
}

void vTaskPrintStatus(void *pvParams)
{
  float temperature;
  bool movementDetected;
  int fanPwm;

  while (1)
  {
    if (
        xQueueReceive(xQueueHandleTemperature, &temperature, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleMovement, &movementDetected, pdMS_TO_TICKS(1000)) &&
        xQueueReceive(xQueueHandleFanSpeed, &fanPwm, pdMS_TO_TICKS(1000)))
    {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.print("°C | Movement Detection: ");
      Serial.print(movementDetected ? "Detected" : "No Movement");
      Serial.print(" | Fan Speed: ");
      Serial.print((fanPwm * 100) / 255);
      Serial.println("%");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
