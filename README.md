# MPU6050
Arduino library for [MPU-6050](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050) Six-Axis (Gyro + Accelerometer) MEMS MotionTracking™ Devices

This library is based on the [great work](https://github.com/kriswiner/MPU6050) by [kriswiner](https://github.com/kriswiner), and re-writen for the simple usage. Also this library is almost compatible with [MPU9250](https://github.com/hideakitai/MPU9250).

## Usage

### Simple Measurement

``` C++
#include "MPU6050.h"

MPU6050 mpu;

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    mpu.setup();
}

void loop()
{
    static uint32_t prev_ms = millis();
    if ((millis() - prev_ms) > 16)
    {
        mpu.update();
        mpu.print();

        Serial.print("roll  (x-forward (north)) : ");
        Serial.println(mpu.getRoll());
        Serial.print("pitch (y-right (east))    : ");
        Serial.println(mpu.getPitch());
        Serial.print("yaw   (z-down (down))     : ");
        Serial.println(mpu.getYaw());

        prev_ms = millis();
    }
}
```

### Calibration

- device should be stay still during accel/gyro calibration

```  C++
#include "MPU6050.h"

MPU6050 mpu;

void setup()
{
    Serial.begin(115200);

    Wire.begin();

    delay(2000);
    mpu.setup();

    delay(5000);

    // calibrate anytime you want to
    mpu.calibrate();

    mpu.printCalibration();
}

void loop()
{
}
```

###

## Other Settings

### AFS, FGS, MFS

#### AFS

`enum class AFS { A2G, A4G, A8G, A16G };`

#### GFS

`enum class GFS { G250DPS, G500DPS, G1000DPS, G2000DPS };`

#### How to change

MPU6050 class is defined as follows.

``` C++
template <
	typename WireType,
	AFS AFSSEL = AFS::A16G,
	GFS GFSSEL = GFS::G2000DPS
>
class MPU6050_;
```

So, please use like

```  C++
class MPU6050_<TwoWire, AFS::A4G, GFS::G500DPS> mpu; // most of Arduino
class MPU6050_<i2c_t3, AFS::A4G, GFS::G500DPS> mpu; // Teensy
```


## License

MIT
 