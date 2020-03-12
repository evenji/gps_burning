#include "altitude_sensor.h"
#include "api_hal_i2c.h"
#include "api_debug.h"
#include "api_os.h"


unsigned short dig_T1;
short dig_T2;
short dig_T3;
unsigned short dig_P1;
short dig_P2;
short dig_P3;
short dig_P4;
short dig_P5;
short dig_P6;
short dig_P7;
short dig_P8;
short dig_P9;

float g_calibration_altitude = 0.0f;

short bmp280_MultipleReadTwo(unsigned char addr)
{
    unsigned char msb, lsb;
    short temp = 0;

    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, addr, 1, &lsb, 1, I2C_DEFAULT_TIME_OUT);
    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, addr + 1, 1, &msb, 1, I2C_DEFAULT_TIME_OUT);

    temp = (short)msb << 8;
    temp |= (short)lsb;

    return temp;
}

long bmp280_MultipleReadThree(unsigned char addr)
{
    unsigned char msb, lsb, xlsb;
    long temp = 0;

    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, addr, 1, &msb, 1, I2C_DEFAULT_TIME_OUT);
    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, addr + 1, 1, &lsb, 1, I2C_DEFAULT_TIME_OUT);
    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, addr + 2, 1, &xlsb, 1, I2C_DEFAULT_TIME_OUT);

    temp = (long)(((unsigned long)msb << 12)|((unsigned long)lsb << 4)|((unsigned long)xlsb >> 4));

    return temp;
}

long bmp280_GetValue(void)
{
    long adc_T;
    long adc_P;
    long var1, var2, t_fine, T, p;

    adc_T = bmp280_MultipleReadThree(BMP280_TEMP_ADDR);
    adc_P = bmp280_MultipleReadThree(BMP280_PRESS_ADDR);

    if(adc_P == 0)
    {
        return 0;
    }

    //Temperature
    var1 = (((double)adc_T)/16384.0-((double)dig_T1)/1024.0)*((double)dig_T2);
    var2 = ((((double)adc_T)/131072.0-((double)dig_T1)/8192.0)*(((double)adc_T)
                /131072.0-((double)dig_T1)/8192.0))*((double)dig_T3);

    t_fine = (unsigned long)(var1+var2);

    T = (var1+var2)/5120.0;

    var1 = ((double)t_fine/2.0)-64000.0;
    var2 = var1*var1*((double)dig_P6)/32768.0;
    var2 = var2 +var1*((double)dig_P5)*2.0;
    var2 = (var2/4.0)+(((double)dig_P4)*65536.0);
    var1 = (((double)dig_P3)*var1*var1/524288.0+((double)dig_P2)*var1)/524288.0;
    var1 = (1.0+var1/32768.0)*((double)dig_P1);
    p = 1048576.0-(double)adc_P;
    p = (p-(var2/4096.0))*6250.0/var1;
    var1 = ((double)dig_P9)*p*p/2147483648.0;
    var2 = p*((double)dig_P8)/32768.0;
    p = p+(var1+var2+((double)dig_P7))/16.0;

    return p;
}

long getAvePressure()
{
    int index = 0;
    long sum = 0;
    long ave = 0;
    for(index = 0; index < 3; index++)
    {
        sum += bmp280_GetValue();
        OS_Sleep(100);
    }
    ave = sum/3;
    return ave;
}

float pressure2Altitude(long pressure)
{
    float altitude = 0.0f;
    altitude = (760.0 - pressure*0.75) * 12;
    return altitude;
}

float ZorePointcalibration()
{
    float calibration_altitude = 0.0f;
    long avePressure = getAvePressure();
    calibration_altitude = pressure2Altitude(avePressure);
    return calibration_altitude;
}

float getAltitude()
{
    float currentAltitue = pressure2Altitude(getAvePressure());
    Trace(1,"current currentAltitue = %f",currentAltitue);
    return (currentAltitue - g_calibration_altitude);
}

void altitudeInit()
{
    unsigned char data = 0xb6;
    
    unsigned char code = 0;
    I2C_ReadMem(I2C_BMP280, BMP280_ADDR, 0xd0, 1, &code, 1, I2C_DEFAULT_TIME_OUT);

    data = 0xff;
    I2C_WriteMem(I2C_BMP280, BMP280_ADDR, 0xf4, 1, &data, 1, I2C_DEFAULT_TIME_OUT);

    data = 0x00;
    I2C_WriteMem(I2C_BMP280, BMP280_ADDR, 0xf5, 1, &data, 1, I2C_DEFAULT_TIME_OUT);

    dig_T1 = bmp280_MultipleReadTwo(0x88);
    dig_T2 = bmp280_MultipleReadTwo(0x8A);
    dig_T3 = bmp280_MultipleReadTwo(0x8C);
    dig_P1 = bmp280_MultipleReadTwo(0x8E);
    dig_P2 = bmp280_MultipleReadTwo(0x90);
    dig_P3 = bmp280_MultipleReadTwo(0x92);
    dig_P4 = bmp280_MultipleReadTwo(0x94);
    dig_P5 = bmp280_MultipleReadTwo(0x96);
    dig_P6 = bmp280_MultipleReadTwo(0x98);
    dig_P7 = bmp280_MultipleReadTwo(0x9A);
    dig_P8 = bmp280_MultipleReadTwo(0x9C);
    dig_P9 = bmp280_MultipleReadTwo(0x9E);

    g_calibration_altitude = ZorePointcalibration();
    Trace(1,"bmp280 calibration value = %f",g_calibration_altitude);
}
