/*
 * @File  app_main.c
 * @Brief An example of SDK's mini system
 * 
 * @Author: Neucrack 
 * @Date: 2017-11-11 16:45:17 
 * @Last Modified by: Neucrack
 * @Last Modified time: 2017-11-11 18:24:56
 */


#include "stdint.h"
#include "stdbool.h"
#include "api_os.h"
#include "api_event.h"
#include "api_debug.h"
#include "api_mqtt.h"
#include "api_network.h"
#include "api_socket.h"
#include "api_key.h"
#include "gps.h"
#include "api_hal_gpio.h"

#include "gps_task.h"
#include "mqtt_task.h"
#include "audio_task.h"
#include "sensor_task.h"
#include "pm_task.h"
#include "led_task.h"

#define AppMain_TASK_STACK_SIZE    (1024 * 2)
#define AppMain_TASK_PRIORITY      1 
HANDLE mainTaskHandle  = NULL;

int16_t csq = 0;


void EventDispatch(API_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case API_EVENT_ID_NETWORK_REGISTERED_HOME: 
        case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
            Trace(1,"gprs register complete");
            setIsGPRSReady(true);
            Network_StartAttach();
            break;
        case API_EVENT_ID_GPS_UART_RECEIVED:
             Trace(1,"received GPS data,length:%d, data:%s,flag:%d",pEvent->param1,pEvent->pParam1,getIsGPRSReady());
            GPS_Update(pEvent->pParam1,pEvent->param1);
            break;
        case API_EVENT_ID_NO_SIMCARD:
            Trace(1,"!!NO SIM CARD%d!!!!",pEvent->param1);
            break;

        case API_EVENT_ID_SYSTEM_READY:
            Trace(1,"system initialize complete");
            break;

        // case API_EVENT_ID_NETWORK_REGISTERED_HOME:
        // case API_EVENT_ID_NETWORK_REGISTERED_ROAMING:
        //     Trace(1,"network register success");
        //     Network_StartAttach();
        //     break;

        case API_EVENT_ID_NETWORK_ATTACHED:
            Trace(1,"network attach success");
            Network_PDP_Context_t context = {
                .apn        ="cmnet",
                .userName   = ""    ,
                .userPasswd = ""
            };
            Network_StartActive(context);
            break;

        case API_EVENT_ID_NETWORK_ACTIVATED:
            Trace(1,"network activate success.."); 
            //OS_ReleaseSemaphore(semMqttStart);
            isStart = true;
            break;
        
        case API_EVENT_ID_SOCKET_CONNECTED:
            Trace(1,"socket connected");
            break;
        
        case API_EVENT_ID_SOCKET_CLOSED:
            Trace(1,"socket closed");
                      
            break;

        case API_EVENT_ID_SIGNAL_QUALITY:
            Trace(1,"CSQ:%d",pEvent->param1);
            csq = pEvent->param1;
            LED_Set_GPRS_Quality(csq);
            break;
        
        case API_EVENT_ID_KEY_DOWN:
            if(pEvent->param1 == KEY_POWER)
            {
                Trace(1,"power key press down now");
                flag_shutdown_status = PM_SD_STATUS_START;
            }
            break;
        case API_EVENT_ID_KEY_UP:
            if(pEvent->param1 == KEY_POWER)
            {
                Trace(1,"power key release now");
                flag_shutdown_status = PM_SD_STATUS_END;
            }
            break;

        case API_EVENT_ID_POWER_ON:
            powerOnCause = pEvent->param1;
            break;

        default:
            break;
    }
}

void AppMainTask(void *pData)
{
    API_Event_t* event=NULL;

    LedTaskHandle = OS_CreateTask(LedTask,
            NULL, NULL, LED_TASK_STACK_SIZE, LED_TASK_PRIORITY, 0, 0, LED_TASK_NAME);
    OS_Sleep(500);
    
    OS_CreateTask(gps_Task,
            NULL, NULL, GPS_TASK_STACK_SIZE, GPS_TASK_PRIORITY, 0, 0, GPS_TASK_NAME);

    OS_Sleep(5000);

    MqttTaskHandle = OS_CreateTask(MqttTask,
        NULL, NULL, MQTT_TASK_STACK_SIZE, MQTT_TASK_PRIORITY, 0, 0, MQTT_TASK_NAME);

    // OS_Sleep(3000);
    // audioTaskHandle = OS_CreateTask(AudioTask,
    //     NULL, NULL, AUDIO_TASK_STACK_SIZE, AUDIO_TASK_PRIORITY, 0, 0, AUDIO_TASK_NAME);
        
    OS_CreateTask(SensorTask,
            NULL, NULL, SENSOR_TASK_STACK_SIZE, SENSOR_TASK_PRIORITY, 0, 0, SENSOR_TASK_NAME);
    OS_Sleep(500);

// OS_Sleep(300);
//     OS_CreateTask(KeepAliveTask,
//         NULL, NULL, KEEP_ALIVE_TASK_STACK_SIZE, KEEP_ALIVE_TASK_PRIORITY, 0, 0, AUDIO_TASK_NAME);

    OS_CreateTask(PMTask,
            NULL, NULL, PM_TASK_STACK_SIZE, PM_TASK_PRIORITY, 0, 0, PM_TASK_NAME);
    OS_Sleep(500);

    while(1)
    {
        if(OS_WaitEvent(mainTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            EventDispatch(event);
            OS_Free(event->pParam1);
            OS_Free(event->pParam2);
            OS_Free(event);
        }
    }
}
void app_Main(void)
{
    mainTaskHandle = OS_CreateTask(AppMainTask ,
        NULL, NULL, AppMain_TASK_STACK_SIZE, AppMain_TASK_PRIORITY, 0, 0, "init Task");
    OS_SetUserMainHandle(&mainTaskHandle);
}