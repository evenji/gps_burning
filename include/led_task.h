#ifndef _LED_TASK_H_
#define _LED_TASK_H_

#define LED_TASK_STACK_SIZE    (2048 * 2)
#define LED_TASK_PRIORITY      9
#define LED_TASK_NAME          "LED Task"

extern HANDLE LedTaskHandle;

void LedTask(void *pData);
void LED_Set_Bat_Quality(int16_t current);
void LED_Set_GPRS_Quality(int16_t current);
void LED_Set_GPS_Quality(int16_t current);

#endif