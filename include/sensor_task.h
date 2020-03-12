#ifndef _SENSOR_TASK_H_
#define _SENSOR_TASK_H_
#include "stdint.h"


#define SENSOR_TASK_STACK_SIZE    (2048 * 2)
#define SENSOR_TASK_PRIORITY      5
#define SENSOR_TASK_NAME          "SENSOR Task"

typedef struct SensorInfo_Tag
{
    uint8_t battery;
    float altitude;
}SensorInfo_T;

void SensorTask(void *pData);
extern SensorInfo_T g_sensorInfo;

#endif