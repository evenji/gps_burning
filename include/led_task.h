#ifndef _LED_TASK_H_
#define _LED_TASK_H_

#define LED_TASK_STACK_SIZE    (2048 * 2)
#define LED_TASK_PRIORITY      9
#define LED_TASK_NAME          "LED Task"

void LedTask(void *pData);

#endif