#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_mqtt.h"
#include "api_network.h"
#include "api_socket.h"

#include "mqtt_task.h"
#include "agent.h"
#include "fault_code.h"

HANDLE MqttTaskHandle = NULL;

HANDLE semMqttStart = NULL;

int reStartTimes = 0;
int restartCnt = 0;

MQTT_Client_t* g_client = NULL;

MQTT_Status_t mqttStatus = MQTT_STATUS_DISCONNECTED;

bool isStart = false;

int flagPublish = 1;

int retErrorCnt = 0;

void MqttInit();

void OnMqttReceived(void* arg, const char* topic, uint32_t payloadLen)
{
    Trace(1,"MQTT received publish data request, topic:%s, payload length:%d",topic,payloadLen);
}

void OnMqttReceiedData(void* arg, const uint8_t* data, uint16_t len, MQTT_Flags_t flags)
{
    Trace(1,"MQTT recieved publish data,  length:%d,data:%s",len,data);
    if(flags == MQTT_FLAG_DATA_LAST)
        Trace(1,"MQTT data is last frame");
    E_FAULT_CODE ret = parserJson(data);
    Trace(1, "parser GPS command status: %d", ret);
}

 void OnMqttSubscribed(void* arg, MQTT_Error_t err)
 {
     if(err != MQTT_ERROR_NONE)
        Trace(1,"MQTT subscribe fail,error code:%d",err);
     else
        Trace(1,"MQTT subscribe success,topic:%s",(const char*)arg);
 }

void OnMqttConnection(MQTT_Client_t *client, void *arg, MQTT_Connection_Status_t status)
{
    Trace(1,"MQTT connection status:%d",status);
    MQTT_Event_t* event = (MQTT_Event_t*)OS_Malloc(sizeof(MQTT_Event_t));
    if(!event)
    {
        Trace(1,"MQTT no memory");
        return ;
    }
    if(status == MQTT_CONNECTION_ACCEPTED)
    {
        Trace(1,"MQTT succeed connect to broker");
        //!!! DO NOT suscribe here(interrupt function), do MQTT suscribe in task, or it will not excute
        event->id = MQTT_EVENT_CONNECTED;
        event->client = client;
        OS_SendEvent(MqttTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    }
    else
    {
        event->id = MQTT_EVENT_DISCONNECTED;
        event->client = client;
        OS_SendEvent(MqttTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
        Trace(1,"MQTT connect to broker fail,error code:%d",status);
    }
    Trace(1,"MQTT OnMqttConnection() end");
}

void StartTimerPublish(uint32_t interval,MQTT_Client_t* client);
void OnPublish(void* arg, MQTT_Error_t err)
{
    MQTT_Event_t* event = (MQTT_Event_t*)OS_Malloc(sizeof(MQTT_Event_t));
    if(!event)
    {
        Trace(1,"MQTT no memory");
        return ;
    }

    if(err == MQTT_ERROR_NONE)
    {
        Trace(1,"MQTT publish success");

            //StartTimerPublish(PUBLISH_INTERVAL_GPS,g_client); 
        //reStartTimes = 0;
    }
    else
    {
        Trace(1,"MQTT publish error, error code:%d",err);
        reStartTimes++;
        if(reStartTimes == 10)
        {
            restartCnt++;
            event->id = MQTT_EVENT_DISCONNECTED;
            event->client = g_client;
            OS_SendEvent(MqttTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
            reStartTimes = 0;
            flagPublish = 0;
            retErrorCnt = 0;
        }
        else
        {
            //StartTimerPublish(PUBLISH_INTERVAL_GPS,g_client);
        }
        
    }
}
void OnTimerPublish(void* param)
{
    MQTT_Error_t err;
    MQTT_Client_t* client = (MQTT_Client_t*)param;

    MQTT_Event_t* event = (MQTT_Event_t*)OS_Malloc(sizeof(MQTT_Event_t));
    if(!event)
    {
        Trace(1,"MQTT no memory");
        return ;
    }

    if(mqttStatus != MQTT_STATUS_CONNECTED)
    {
        Trace(1,"MQTT not connected to broker! can not publish");
        return;
    }
    char* publish_payload = getDevInfoJsonStr();
    Trace(1,"MQTT OnTimerPublish payload = %s", publish_payload);
    if(flagPublish)
    {
        err = MQTT_Publish(client,PUBLISH_TOPIC,publish_payload,strlen(publish_payload),1,0,0,OnPublish,NULL);
        if(err != MQTT_ERROR_NONE)
        {
            Trace(1,"2222MQTT publish error, error code:%d",err);
            retErrorCnt++;
            if( retErrorCnt == 15 )
            {
                restartCnt++;
                event->id = MQTT_EVENT_DISCONNECTED;
                event->client = g_client;
                OS_SendEvent(MqttTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
                reStartTimes = 0;
                flagPublish = 0;
                retErrorCnt = 0;
            }
        }else
        {
            retErrorCnt = 0;
        }
        
    }

    StartTimerPublish(PUBLISH_INTERVAL_GPS,client);
}

void StartTimerPublish(uint32_t interval,MQTT_Client_t* client)
{
    OS_StartCallbackTimer(MqttTaskHandle,interval,OnTimerPublish,(void*)client);
}

void SecondTaskEventDispatch(MQTT_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case MQTT_EVENT_CONNECTED:
            mqttStatus = MQTT_STATUS_CONNECTED;
            Trace(1,"1MQTT connected, now subscribe topic:%s",SUBSCRIBE_TOPIC);
            MQTT_Error_t err;
            MQTT_SetInPubCallback(pEvent->client, OnMqttReceived, OnMqttReceiedData, NULL);
            err = MQTT_Subscribe(pEvent->client,SUBSCRIBE_TOPIC,0,OnMqttSubscribed,(void*)SUBSCRIBE_TOPIC);
            if(err != MQTT_ERROR_NONE)
                Trace(1,"MQTT subscribe error, error code:%d",err);
            StartTimerPublish(PUBLISH_INTERVAL_GPS,pEvent->client);
            break;
        case MQTT_EVENT_DISCONNECTED:
            //MQTT_Disconnect
            MQTT_Disconnect(g_client);
            OS_Sleep(1000);
            MqttInit();
            mqttStatus = MQTT_STATUS_NEED_RESTART;
            break;
        default:
            break;
    }
}

void MqttInit()
{
    
    mqttStatus = MQTT_STATUS_START_CONNECT;
    Trace(1,"start mqtt test start");
    // semMqttStart = OS_CreateSemaphore(0);
    // OS_WaitForSemaphore(semMqttStart,OS_WAIT_FOREVER);
    // OS_DeleteSemaphore(semMqttStart);
    //OS_Sleep(10000);
    while(isStart == false)
    {
        OS_Sleep(1000);
        Trace(1,"waiting...");
    }
    reStartTimes++;
    Trace(1,"start mqtt test");
    g_client = MQTT_ClientNew();
    MQTT_Connect_Info_t ci;
    MQTT_Error_t err;
    memset(&ci,0,sizeof(MQTT_Connect_Info_t));
    ci.client_id = CLIENT_ID;
    ci.client_user = CLIENT_USER;
    ci.client_pass = CLIENT_PASS;
    ci.keep_alive = 60;
    ci.clean_session = 1;
    ci.use_ssl = false;
    flagPublish = 1;

    err = MQTT_Connect(g_client,BROKER_IP,BROKER_PORT,OnMqttConnection,NULL,&ci);
    if(err != MQTT_ERROR_NONE)
        Trace(1,"MQTT connect fail,error code:%d",err);
}

void MqttTask(void *pData)
{
    MQTT_Event_t* event=NULL;
    MqttInit();
    
    while(1)
    {
        if(OS_WaitEvent(MqttTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            SecondTaskEventDispatch(event);
            OS_Free(event);
        }
    }
}