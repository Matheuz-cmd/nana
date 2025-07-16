#include <Servo.h>
#include "Pins.h"

Servo servo;

void servoSetup()
{
  servo.attach(SERVO_PIN, 2, 45, 120, 500, 2400, 200);

  if (servo.attached()){
    Serial.println("[SG 90 SERVO SETUP] Servo Setup Sucessful!");
  } else {
    Serial.println("[SG 90 SERVO SETUP] Servo Setup Failed!");
  }
}

void startSwing()
{
  for (int angle = 90; angle <= 120; angle++)
  {
    servo.write(angle);
    delay(20);
  }
}