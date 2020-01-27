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
#include "gps.h"

#include "gps_task.h"
#include "mqtt_task.h"

#define AppMain_TASK_STACK_SIZE    (1024 * 2)
#define AppMain_TASK_PRIORITY      1 
HANDLE mainTaskHandle  = NULL;


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
            OS_ReleaseSemaphore(semMqttStart);
            break;
        
        case API_EVENT_ID_SOCKET_CONNECTED:
            Trace(1,"socket connected");
            break;
        
        case API_EVENT_ID_SOCKET_CLOSED:
            Trace(1,"socket closed");
                      
            break;

        case API_EVENT_ID_SIGNAL_QUALITY:
            Trace(1,"CSQ:%d",pEvent->param1);
            break;

        default:
            break;
    }
}


void AppMainTask(void *pData)
{
    API_Event_t* event=NULL;
    
    OS_CreateTask(gps_Task,
            NULL, NULL, GPS_TASK_STACK_SIZE, GPS_TASK_PRIORITY, 0, 0, GPS_TASK_NAME);

    OS_Sleep(5000);

    MqttTaskHandle = OS_CreateTask(MqttTask,
        NULL, NULL, MQTT_TASK_STACK_SIZE, MQTT_TASK_PRIORITY, 0, 0, MQTT_TASK_NAME);


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