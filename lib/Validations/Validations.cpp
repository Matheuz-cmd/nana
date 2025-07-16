#include "math.h"

bool validateTemperatureReading(float temperature)
{
    return !isnan(temperature);
}

bool validateMovementReading(bool movementDetected)
{
    return movementDetected == true || movementDetected == false;
}

bool validateNoiseLevelReading(int noiseLevel)
{
    return noiseLevel >= 1200;
}