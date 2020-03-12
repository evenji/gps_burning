#ifndef _ALTITUDE_SENSOR_H_
#define _ALTITUDE_SENSOR_H_

#define I2C_BMP280 I2C2

#define BMP280_ADDR         0x76
#define BMP280_TEMP_ADDR    0xfa
#define BMP280_PRESS_ADDR   0xf7

void  altitudeInit();
float getAltitude();

#endif