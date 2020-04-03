#include "MPU6050.h"
#include "eeprom_utils.h"

MPU6050 imu;

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    imu.setup(Wire);

    delay(5000);

    // calibrate when you want to
    imu.calibrate();

    // save to eeprom
    saveCalibration();

    // load from eeprom
    loadCalibration();

    imu.printCalibration();
}

void loop()
{
}
