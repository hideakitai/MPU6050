#include "EEPROM.h"
#include "MPU6050.h"

const uint8_t EEPROM_SIZE = 1 + sizeof(float) * 3 * 4;
bool b_calibrated = false;
extern MPU6050 imu;

enum EEP_ADDR
{
    EEP_CALIB_FLAG = 0x00,
    EEP_ACC_BIAS = 0x01,
    EEP_GYRO_BIAS = 0x0D,
};

void saveCalibration(bool b_save = true)
{
    EEPROM.writeByte(EEP_CALIB_FLAG, 1);
    EEPROM.writeFloat(EEP_ACC_BIAS + 0, imu.getAccBias(0));
    EEPROM.writeFloat(EEP_ACC_BIAS + 4, imu.getAccBias(1));
    EEPROM.writeFloat(EEP_ACC_BIAS + 8, imu.getAccBias(2));
    EEPROM.writeFloat(EEP_GYRO_BIAS + 0, imu.getGyroBias(0));
    EEPROM.writeFloat(EEP_GYRO_BIAS + 4, imu.getGyroBias(1));
    EEPROM.writeFloat(EEP_GYRO_BIAS + 8, imu.getGyroBias(2));
    if (b_save) EEPROM.commit();
}

void loadCalibration()
{
    if (EEPROM.readByte(EEP_CALIB_FLAG) == 0x01)
    {
        Serial.println("calibrated? : YES");
        Serial.println("load calibrated values");
        imu.setAccBias(0, EEPROM.readFloat(EEP_ACC_BIAS + 0));
        imu.setAccBias(1, EEPROM.readFloat(EEP_ACC_BIAS + 4));
        imu.setAccBias(2, EEPROM.readFloat(EEP_ACC_BIAS + 8));
        imu.setGyroBias(0, EEPROM.readFloat(EEP_GYRO_BIAS + 0));
        imu.setGyroBias(1, EEPROM.readFloat(EEP_GYRO_BIAS + 4));
        imu.setGyroBias(2, EEPROM.readFloat(EEP_GYRO_BIAS + 8));
    }
    else
    {
        Serial.println("calibrated? : NO");
        Serial.println("load default values");
        imu.setAccBias(0, +0.005);
        imu.setAccBias(1, -0.008);
        imu.setAccBias(2, -0.001);
        imu.setGyroBias(0, +1.5);
        imu.setGyroBias(1, -0.5);
        imu.setGyroBias(2, +0.7);
    }
}

void clearCalibration()
{
    for (size_t i = 0; i < EEPROM_SIZE; ++i)
        EEPROM.writeByte(i, 0xFF);
    EEPROM.commit();
}

bool isCalibrated()
{
    return (EEPROM.readByte(EEP_CALIB_FLAG) == 0x01);
}

void printCalibration()
{
    Serial.println("< calibration parameters >");
    Serial.print("calibrated? : ");
    Serial.println(EEPROM.readByte(EEP_CALIB_FLAG) ? "YES" : "NO");
    Serial.print("acc bias x  : ");
    Serial.println(EEPROM.readFloat(EEP_ACC_BIAS + 0) * 1000.f);
    Serial.print("acc bias y  : ");
    Serial.println(EEPROM.readFloat(EEP_ACC_BIAS + 4) * 1000.f);
    Serial.print("acc bias z  : ");
    Serial.println(EEPROM.readFloat(EEP_ACC_BIAS + 8) * 1000.f);
    Serial.print("gyro bias x : ");
    Serial.println(EEPROM.readFloat(EEP_GYRO_BIAS + 0));
    Serial.print("gyro bias y : ");
    Serial.println(EEPROM.readFloat(EEP_GYRO_BIAS + 4));
    Serial.print("gyro bias z : ");
    Serial.println(EEPROM.readFloat(EEP_GYRO_BIAS + 8));
}

void printBytes()
{
    for (size_t i = 0; i < EEPROM_SIZE; ++i)
        Serial.println(EEPROM.readByte(i), HEX);
}

void setupEEPROM()
{
    Serial.println("EEPROM start");
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("EEPROM start failed");
    }

    b_calibrated = isCalibrated();
    if (!b_calibrated)
    {
        Serial.println("Need Calibration!!");
    }
    Serial.println("EEPROM calibration value is : ");
    printCalibration();
    Serial.println("Loaded calibration value is : ");
    loadCalibration();
}
