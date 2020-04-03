#include "MPU6050.h"

MPU6050 imu;

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    imu.setup();

    delay(5000);

    // calibrate anytime you want to
    imu.calibrate();

    imu.printCalibration();
}

void loop()
{
}
