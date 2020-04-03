#include "MPU6050.h"

MPU6050 imu;

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    imu.setup();
}

void loop()
{
    static uint32_t prev_ms = millis();
    if ((millis() - prev_ms) > 16)
    {
        imu.update();
        imu.print();

        Serial.print("roll  (x-forward (north)) : ");
        Serial.println(imu.getRoll());
        Serial.print("pitch (y-right (east))    : ");
        Serial.println(imu.getPitch());
        Serial.print("yaw   (z-down (down))     : ");
        Serial.println(imu.getYaw());

        prev_ms = millis();
    }
}

