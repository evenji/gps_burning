#ifndef _GPS_TASK_H_
#define _GPS_TASK_H_

#include "gprs.h"
#include "gps_parse.h"
#include "fault_code.h"

#define GPS_TASK_STACK_SIZE (2048 * 2)
#define GPS_TASK_PRIORITY   1
#define GPS_TASK_NAME       "GPS Task"

extern GPS_Info_t* gpsInfo;

E_FAULT_CODE getGPSDate(char *date);
void getGPSInfo(char* longitude, char* latitude, char *altitude, char* speed, int* direction);
void gps_Task(void *pData);

#endif