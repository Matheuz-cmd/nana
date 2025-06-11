#include <DHT.h>

#define DHTPIN 4           // Pino do DHT11
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int pwmPin = 23;     // PWM azul (controle da fan)
const int tachoPin = 34;   // Fio verde (leitura do tacômetro)

volatile int pulseCount = 0;
unsigned long lastRPMTime = 0;

void IRAM_ATTR countPulse() {
  pulseCount++;
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // PWM setup: canal 0, frequência 25 kHz, 8 bits
  ledcSetup(0, 25000, 8);
  ledcAttachPin(pwmPin, 0);  // PWM no fio azul da fan

  // Tacômetro
  pinMode(tachoPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(tachoPin), countPulse, FALLING);
}

void loop() {
  float temp = dht.readTemperature();

  if (isnan(temp)) {
    Serial.println("Erro ao ler o DHT11.");
    delay(2000);
    return;
  }

  int pwmValue;
  if (temp < 30.0) {
    pwmValue = 0;   // Fan desligada
  } else {
    pwmValue = 255; // Fan em velocidade máxima
  }

  ledcWrite(0, 255 - pwmValue);  // PWM invertido (ativo em LOW)

  // Cálculo de RPM
  unsigned long currentTime = millis();
  if (currentTime - lastRPMTime >= 1000) {
    int pulses = pulseCount;
    pulseCount = 0;

    int rpm = (pulses * 60) / 2;
    Serial.print("Temperatura: ");
    Serial.print(temp, 2);
    Serial.print(" °C | PWM: ");
    Serial.print(pwmValue);
    Serial.print(" | RPM: ");
    Serial.println(rpm);

    lastRPMTime = currentTime;
  }

  delay(100);
}
