#include <Servo.h>
#include "Pins.h"

Servo servo;
volatile int angle = 90;

void servoSetup()
{
  servo.attach(SERVO_PIN);
  servo.write(angle);
}

void startSwing()
{
  for (angle = 75; angle <= 105; angle++)
  {
    servo.write(angle);
    delay(40); 
  }

  for (angle = 105; angle >= 75; angle--)
  {
    servo.write(angle);
    delay(40);
  }
}