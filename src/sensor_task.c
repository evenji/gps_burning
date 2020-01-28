
#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_hal_adc.h"
#include "sensor_task.h"

#define BATTERY_VOLTAGE_MV_MAX 4150
#define BATTERY_VOLTAGE_MV_MIN 3000

SensorInfo_T g_sensorInfo = {.battery = 100};

void getBatteryInfo()
{
    uint16_t value = 0, batteryVoltagemv = 0;
    if(ADC_Read(ADC_CHANNEL_0, &value, &batteryVoltagemv))
    {
        batteryVoltagemv *= 3;
        if( batteryVoltagemv >= BATTERY_VOLTAGE_MV_MAX)
        {
            g_sensorInfo.battery = 100;
        }
        else if( batteryVoltagemv <= BATTERY_VOLTAGE_MV_MIN)
        {
            g_sensorInfo.battery = 0;
        }
        else
        {
            g_sensorInfo.battery = (uint32_t)(batteryVoltagemv - BATTERY_VOLTAGE_MV_MIN) *100 / (BATTERY_VOLTAGE_MV_MAX - BATTERY_VOLTAGE_MV_MIN);
        }
        Trace(1,"ADC value:%d, %dmV battery=%d",value,batteryVoltagemv,g_sensorInfo.battery);
    }
}

void SensorTask(void *pData)
{
    ADC_Config_t config = {
        .channel = ADC_CHANNEL_0,
        .samplePeriod = ADC_SAMPLE_PERIOD_100MS
    };
    ADC_Init(config);

    Trace(1, "Sensor Task");
    while(1)
    {
        getBatteryInfo();
        OS_Sleep(10000);
    }
}