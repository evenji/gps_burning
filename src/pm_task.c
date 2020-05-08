#include "stdint.h"
#include "stdbool.h"
#include "api_os.h"
#include "api_event.h"
#include "api_debug.h"

#include "pm_task.h"
#include "altitude_sensor.h"

PM_SD_STATUS flag_shutdown_status = PM_SD_STATUS_INIT;
Power_On_Cause_t powerOnCause = POWER_ON_CAUSE_MAX;

void PMTask(void *pData)
{
    Trace(1, "PM Task");
    uint16_t shtudown_cnt = 0;
    while(1)
    {
        if(flag_shutdown_status == PM_SD_STATUS_START)
        {
            shtudown_cnt++;
            if(shtudown_cnt == 18)
            {
                Trace(1, "PM Task: Press Power Button, will Shutdown.");
                sensorAltitudeSleep();
                OS_Sleep(100);
                PM_ShutDown();
            }
        }
        else
        {
            shtudown_cnt = 0;
        }
        
        OS_Sleep(200);
    }
}