#pragma once
#ifndef ARDUINO_MPU_6050_H
#define ARDUINO_MPU_6050_H

#include <Wire.h>
#include "MPU6050Library/MPU6050.h"

template <typename WireType, AFS AFSSEL = AFS::A16G, GFS GFSSEL = GFS::G2000DPS>
class MPU6050_
{
    MPU6050lib<WireType, AFSSEL, GFSSEL> mpu;

    float aRes, gRes; // scale resolutions per LSB for the sensors
    // Pin definitions
    int16_t accelCount[3];  // Stores the 16-bit signed accelerometer sensor output
    float a[3];       // Stores the real accel value in g's
    int16_t gyroCount[3];   // Stores the 16-bit signed gyro sensor output
    float g[3];       // Stores the real gyro value in degrees per seconds
    float gyroBias[3] = {0, 0, 0}, accelBias[3] = {0, 0, 0}; // Bias corrections for gyro and accelerometer
    int16_t tempCount;   // Stores the real internal chip temperature in degrees Celsius
    float temperature;
    float SelfTest[6];
    float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};            // vector to hold quaternion
    float pitch, yaw, roll;
    // parameters for 6 DoF sensor fusion calculations
    float GyroMeasError = PI * (40.0f / 180.0f);     // gyroscope measurement error in rads/s (start at 60 deg/s), then reduce after ~10 s to 3
    float beta = sqrt(3.0f / 4.0f) * GyroMeasError;  // compute beta
    float GyroMeasDrift = PI * (2.0f / 180.0f);      // gyroscope measurement drift in rad/s/s (start at 0.0 deg/s/s)
    float zeta = sqrt(3.0f / 4.0f) * GyroMeasDrift;  // compute zeta, the other free parameter in the Madgwick scheme usually set to a small or zero value
    float deltat = 0.0f;                              // integration interval for both filter schemes
    uint32_t lastUpdate = 0, firstUpdate = 0;         // used to calculate integration interval
    uint32_t Now = 0;                                 // used to calculate integration interval

    static constexpr uint8_t WHOAMI_DEFAULT_VALUE {0x68};

public:

    bool isConnected()
    {
        // Read the WHO_AM_I register, this is a good test of communication
        uint8_t c = mpu.readByte(MPU6050_ADDRESS, WHO_AM_I_MPU6050);  // Read WHO_AM_I register for MPU-6050
        Serial.print("I AM ");
        Serial.print(c, HEX);
        Serial.print(" I Should Be ");
        Serial.println(WHOAMI_DEFAULT_VALUE, HEX);
        return (c == WHOAMI_DEFAULT_VALUE); // WHO_AM_I should always be 0x68
    }

    bool selfTest()
    {
        mpu.MPU6050SelfTest(SelfTest); // Start by performing self test and reporting values
        //    Serial.print("x-axis self test: acceleration trim within : "); Serial.print(SelfTest[0],1); Serial.println("% of factory value");
        //    Serial.print("y-axis self test: acceleration trim within : "); Serial.print(SelfTest[1],1); Serial.println("% of factory value");
        //    Serial.print("z-axis self test: acceleration trim within : "); Serial.print(SelfTest[2],1); Serial.println("% of factory value");
        //    Serial.print("x-axis self test: gyration trim within : "); Serial.print(SelfTest[3],1); Serial.println("% of factory value");
        //    Serial.print("y-axis self test: gyration trim within : "); Serial.print(SelfTest[4],1); Serial.println("% of factory value");
        //    Serial.print("z-axis self test: gyration trim within : "); Serial.print(SelfTest[5],1); Serial.println("% of factory value");

        return ((SelfTest[0] < 1.0f) && (SelfTest[1] < 1.0f) && (SelfTest[2] < 1.0f) && (SelfTest[3] < 1.0f) && (SelfTest[4] < 1.0f) && (SelfTest[5] < 1.0f));
    }

    void calibrate()
    {
        mpu.calibrateMPU6050(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers
    }

    bool setup(WireType& w = Wire)
    {
        mpu.setup(w);

        if (isConnected()) // WHO_AM_I should always be 0x68
        {
            Serial.println("MPU6050 is online...");

            if (selfTest())
            {
                Serial.println("Pass Selftest!");
                calibrate();

                mpu.initMPU6050();
                Serial.println("MPU6050 initialized for active data mode...."); // Initialize device for active mode read of acclerometer, gyroscope, and temperature

                return true;
            }
            else
            {
                Serial.println("Could not connect to MPU6050");
            }
        }
        return false;
    }


    bool available()
    {
        return (mpu.readByte(MPU6050_ADDRESS, INT_STATUS) & 0x01); // check if data ready interrupt
    }

    float getRoll() const { return roll; }
    float getPitch() const { return pitch; }
    float getYaw() const { return yaw; }
    float getQuaternion(uint8_t i) const { return (i < 4) ? q[i] : 0.f; }

    float getAcc(uint8_t i) const { return (i < 3) ? a[i] : 0.f; }
    float getGyro(uint8_t i) const { return (i < 3) ? g[i] : 0.f; }
    float getAccBias(uint8_t i) const { return (i < 3) ? accelBias[i] : 0.f; }
    float getGyroBias(uint8_t i) const { return (i < 3) ? gyroBias[i] : 0.f; }

    void setAccBias(uint8_t i, float v) { if (i < 3) accelBias[i] = v; }
    void setGyroBias(uint8_t i, float v) { if (i < 3) gyroBias[i] = v; }

    void print() const
    {
        printRawData();
        printRollPitchYaw();
        printCalibration();
    }

    void printRawData() const
    {
        Serial.println(" x\t  y\t  z  ");

        Serial.print((int)(1000 * a[0])); Serial.print('\t');
        Serial.print((int)(1000 * a[1])); Serial.print('\t');
        Serial.print((int)(1000 * a[2]));
        Serial.println(" mg");

        Serial.print((int)(g[0])); Serial.print('\t');
        Serial.print((int)(g[1])); Serial.print('\t');
        Serial.print((int)(g[2]));
        Serial.println(" o/s");
    }

    void printRollPitchYaw() const
    {
        Serial.print("Yaw, Pitch, Roll: ");
        Serial.print(yaw, 2);
        Serial.print(", ");
        Serial.print(pitch, 2);
        Serial.print(", ");
        Serial.println(roll, 2);
    }

    void printCalibration() const
    {
        Serial.println("MPU6050 bias");
        Serial.println(" x\t  y\t  z  ");
        Serial.print((int)(1000 * accelBias[0])); Serial.print('\t');
        Serial.print((int)(1000 * accelBias[1])); Serial.print('\t');
        Serial.print((int)(1000 * accelBias[2]));
        Serial.println(" mg");

        Serial.print(gyroBias[0], 1); Serial.print('\t');
        Serial.print(gyroBias[1], 1); Serial.print('\t');
        Serial.print(gyroBias[2], 1);
        Serial.println(" o/s");
    }

    void update()
    {
        // If data ready bit set, all data registers have new data
        if (available())
        {
            mpu.readAccelData(accelCount);  // Read the x/y/z adc values
            aRes = mpu.getAres();

            // Now we'll calculate the accleration value into actual g's
            a[0] = (float)accelCount[0] * aRes; // get actual g value, this depends on scale being set
            a[1] = (float)accelCount[1] * aRes;
            a[2] = (float)accelCount[2] * aRes;

            mpu.readGyroData(gyroCount);  // Read the x/y/z adc values
            gRes = mpu.getGres();

            // Calculate the gyro value into actual degrees per second
            g[0] = (float)gyroCount[0] * gRes; // get actual gyro value, this depends on scale being set
            g[1] = (float)gyroCount[1] * gRes;
            g[2] = (float)gyroCount[2] * gRes;

            tempCount = mpu.readTempData();  // Read the x/y/z adc values
            temperature = ((float) tempCount) / 340. + 36.53; // Temperature in degrees Centigrade

            // Pass gyro rate as rad/s
            MadgwickQuaternionUpdate(a[0], a[1], a[2], g[0] * PI / 180.0f, g[1] * PI / 180.0f, g[2] * PI / 180.0f);

            // Define output variables from updated quaternion---these are Tait-Bryan angles, commonly used in aircraft orientation.
            // In this coordinate system, the positive z-axis is down toward Earth.
            // Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
            // Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
            // Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
            // These arise from the definition of the homogeneous rotation matrix constructed from quaternions.
            // Tait-Bryan angles as well as Euler angles are non-commutative; that is, the get the correct orientation the rotations must be
            // applied in the correct order which for this configuration is yaw, pitch, and then roll.
            // For more see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles which has additional links.
            yaw   = atan2(2.0f * (q[1] * q[2] + q[0] * q[3]), q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3]);
            pitch = -asin(2.0f * (q[1] * q[3] - q[0] * q[2]));
            roll  = atan2(2.0f * (q[0] * q[1] + q[2] * q[3]), q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3]);

            pitch *= 180.0f / PI;
            yaw   *= 180.0f / PI;
            roll  *= 180.0f / PI;
        }
    }

private:

    // Implementation of Sebastian Madgwick's "...efficient orientation filter for... inertial/magnetic sensor arrays"
    // (see http://www.x-io.co.uk/category/open-source/ for examples and more details)
    // which fuses acceleration and rotation rate to produce a quaternion-based estimate of relative
    // device orientation -- which can be converted to yaw, pitch, and roll. Useful for stabilizing quadcopters, etc.
    // The performance of the orientation filter is at least as good as conventional Kalman-based filtering algorithms
    // but is much less computationally intensive---it can be performed on a 3.3 V Pro Mini operating at 8 MHz!
    void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz)
    {
        float q1 = q[0], q2 = q[1], q3 = q[2], q4 = q[3];         // short name local variable for readability
        float norm;                                               // vector norm
        float f1, f2, f3;                                         // objetive funcyion elements
        float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33; // objective function Jacobian elements
        float qDot1, qDot2, qDot3, qDot4;
        float hatDot1, hatDot2, hatDot3, hatDot4;
        float gerrx, gerry, gerrz, gbiasx, gbiasy, gbiasz;        // gyro bias error

        // Auxiliary variables to avoid repeated arithmetic
        float _halfq1 = 0.5f * q1;
        float _halfq2 = 0.5f * q2;
        float _halfq3 = 0.5f * q3;
        float _halfq4 = 0.5f * q4;
        float _2q1 = 2.0f * q1;
        float _2q2 = 2.0f * q2;
        float _2q3 = 2.0f * q3;
        float _2q4 = 2.0f * q4;
        // float _2q1q3 = 2.0f * q1 * q3;
        // float _2q3q4 = 2.0f * q3 * q4;

        // Normalise accelerometer measurement
        norm = sqrt(ax * ax + ay * ay + az * az);
        if (norm == 0.0f) return; // handle NaN
        norm = 1.0f/norm;
        ax *= norm;
        ay *= norm;
        az *= norm;

        // Compute the objective function and Jacobian
        f1 = _2q2 * q4 - _2q1 * q3 - ax;
        f2 = _2q1 * q2 + _2q3 * q4 - ay;
        f3 = 1.0f - _2q2 * q2 - _2q3 * q3 - az;
        J_11or24 = _2q3;
        J_12or23 = _2q4;
        J_13or22 = _2q1;
        J_14or21 = _2q2;
        J_32 = 2.0f * J_14or21;
        J_33 = 2.0f * J_11or24;

        // Compute the gradient (matrix multiplication)
        hatDot1 = J_14or21 * f2 - J_11or24 * f1;
        hatDot2 = J_12or23 * f1 + J_13or22 * f2 - J_32 * f3;
        hatDot3 = J_12or23 * f2 - J_33 *f3 - J_13or22 * f1;
        hatDot4 = J_14or21 * f1 + J_11or24 * f2;

        // Normalize the gradient
        norm = sqrt(hatDot1 * hatDot1 + hatDot2 * hatDot2 + hatDot3 * hatDot3 + hatDot4 * hatDot4);
        hatDot1 /= norm;
        hatDot2 /= norm;
        hatDot3 /= norm;
        hatDot4 /= norm;

        // Compute estimated gyroscope biases
        gerrx = _2q1 * hatDot2 - _2q2 * hatDot1 - _2q3 * hatDot4 + _2q4 * hatDot3;
        gerry = _2q1 * hatDot3 + _2q2 * hatDot4 - _2q3 * hatDot1 - _2q4 * hatDot2;
        gerrz = _2q1 * hatDot4 - _2q2 * hatDot3 + _2q3 * hatDot2 - _2q4 * hatDot1;

        // Compute and remove gyroscope biases
        // gbiasx += (gerrx * deltat * zeta);
        // gbiasy += (gerry * deltat * zeta);
        // gbiasz += (gerrz * deltat * zeta);
        gbiasx = (gerrx * deltat * zeta);
        gbiasy = (gerry * deltat * zeta);
        gbiasz = (gerrz * deltat * zeta);
        gx -= gbiasx;
        gy -= gbiasy;
        gz -= gbiasz;

        // Compute the quaternion derivative
        qDot1 = -_halfq2 * gx - _halfq3 * gy - _halfq4 * gz;
        qDot2 =  _halfq1 * gx + _halfq3 * gz - _halfq4 * gy;
        qDot3 =  _halfq1 * gy - _halfq2 * gz + _halfq4 * gx;
        qDot4 =  _halfq1 * gz + _halfq2 * gy - _halfq3 * gx;

        // Compute then integrate estimated quaternion derivative
        q1 += (qDot1 -(beta * hatDot1)) * deltat;
        q2 += (qDot2 -(beta * hatDot2)) * deltat;
        q3 += (qDot3 -(beta * hatDot3)) * deltat;
        q4 += (qDot4 -(beta * hatDot4)) * deltat;

        // Normalize the quaternion
        norm = sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4);    // normalise quaternion
        norm = 1.0f/norm;
        q[0] = q1 * norm;
        q[1] = q2 * norm;
        q[2] = q3 * norm;
        q[3] = q4 * norm;
    }
};

using MPU6050 = MPU6050_<TwoWire, AFS::A16G, GFS::G2000DPS>;

#endif // ARDUINO_MPU_6050_H
