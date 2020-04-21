#ifndef _PM_TASK_H_
#define _PM_TASK_H_

#include "api_hal_pm.h"

#define PM_TASK_STACK_SIZE    (2048 * 2)
#define PM_TASK_PRIORITY      8
#define PM_TASK_NAME          "PM Task"

typedef enum{
    PM_SD_STATUS_INIT,
    PM_SD_STATUS_START,
    PM_SD_STATUS_END
}PM_SD_STATUS;   // shutdown status

void PMTask(void *pData);
extern PM_SD_STATUS flag_shutdown_status;
extern Power_On_Cause_t powerOnCause;

#endif