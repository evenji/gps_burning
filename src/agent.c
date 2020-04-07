#include "agent.h"
#include "gps_task.h"
#include "sys_config.h"
#include "api_debug.h"
#include "api_os.h"
#include "cJSON.h"
#include "audio_task.h"
#include "sensor_task.h"

#define MQTT_COMMAND_TYPE "command_type"
#define MQTT_COMMAND_TYPE_PLAY_AUDIO "play_audio"

#define MQTT_COMMAND_CONTENT "command_content"

extern int reStartTimes;
extern int restartCnt;
extern int16_t csq;

uint32_t package_number = 0;
typedef struct DevInfo_Tag
{
    char version[16];
    char sn[16];
    uint32_t package_number;
    char date[32];
    char longitude[16];
    char latitude[16];
    char altitude[8];
    uint32_t stars_tracked;
    char speed[8];
    int direction;
    char battery[8];
    uint16_t csq;
    uint32_t fault_code;
}DevInfo_T;

char* devInfoJson = "{\"reported\":{\"version\":\"%s\",\"serial_number\":\"%s\",\"package_number\":%d,\"date\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"altitude\":\"%s\",\"stars_tracked\":\"%d\",\"speed\":\"%s\",\"direction\":%d,\"battery\":\"%s\",\"csq\":%d,\"fault_code\":%d}}";

uint8_t buffer[300];
char * createDevInfoJsonStr(DevInfo_T *info)
{    
    snprintf(buffer, sizeof(buffer), devInfoJson, info->version, 
                info->sn, info->package_number, info->date,info->longitude,info->latitude,
                info->altitude, info->stars_tracked, info->speed, info->direction,info->battery, info->csq, info->fault_code);
    return &buffer[0];
}

char* getDevInfoJsonStr()
{
    DevInfo_T devinfo;
    char batterystr[8];
    E_FAULT_CODE ret_gps;
    strcpy(devinfo.version, SOFTWARE_VERSION);
    strcpy(devinfo.sn, SERIAL_NUMBER);
    devinfo.package_number = package_number;
    ret_gps = getGPSDate(devinfo.date);
    getGPSInfo(devinfo.longitude, devinfo.latitude, devinfo.altitude, devinfo.speed, &(devinfo.direction), &(devinfo.stars_tracked));

    sprintf(batterystr, "%d", g_sensorInfo.battery);
    sprintf(devinfo.altitude, "%.1f", g_sensorInfo.altitude);
    strcpy(devinfo.battery, batterystr);
    //Trace(1,"Get GPS status = %d", ret_gps);
    // if(ret_gps == RET_GPS_NO_SIGNAL)
    // {
    //     devinfo.fault_code = RET_GPS_NO_SIGNAL;
    // }else
    // {
    //     devinfo.fault_code = 0;
    // }
    devinfo.fault_code = restartCnt;
    devinfo.csq = csq;
    //Trace(1,"Get devinfo.fault_code = %d", devinfo.fault_code);
    createDevInfoJsonStr(&devinfo);
    package_number++;
    return &buffer[0];
}

AudioControl_T g_audioControl;

E_FAULT_CODE getAudioControl(char *command_content)
{
    if(command_content ==NULL)
    {
        return RET_NULL_POINTER;
    }

    cJSON * cj2 = cJSON_Parse(command_content);
    if(cj2 == NULL){
        Trace(1,"Get Json Fialed");
        return RET_NULL_POINTER;
    }

    cJSON *index = (cJSON_GetObjectItem(cj2, "index"));
    cJSON *volume = (cJSON_GetObjectItem(cj2, "volume"));
    cJSON* isLoop = (cJSON_GetObjectItem(cj2, "isLoop"));

    if(index == NULL || volume == NULL || isLoop == NULL )
    {
        return RET_NULL_POINTER;
    }

    g_audioControl.index = atoi(index->valuestring);
    g_audioControl.volume = atoi(volume->valuestring);
    g_audioControl.isLoop = atoi(isLoop->valuestring);

    Trace(1, "Get Audio Control Success, index = %d, volume = %d, isLoop = %d isLoop = %s", g_audioControl.index, g_audioControl.volume, g_audioControl.isLoop, isLoop->valuestring);
    Audio_Event_t* event = (Audio_Event_t*)OS_Malloc(sizeof(Audio_Event_t));
    if(!event)
    {
        cJSON_Delete(cj2);
        Trace(1,"Audio no memory");
        return ;
    }
    event->id = AUDIO_EVENT_PLAY;
    event->audioControl = &g_audioControl;
    OS_SendEvent(audioTaskHandle,event,OS_TIME_OUT_WAIT_FOREVER,OS_EVENT_PRI_NORMAL);
    cJSON_Delete(cj2);
    return RET_OK;
    
}

E_FAULT_CODE parserGPSCommand(char* command_type, char* command_content)
{
    if(command_type == NULL || command_content == NULL)
    {
        return RET_NULL_POINTER;
    }

    if(!strcmp(command_type, MQTT_COMMAND_TYPE_PLAY_AUDIO))
    {
        Trace(1, "get play audio command %s", command_content);
        getAudioControl(command_content);
    }else
    {
        Trace(1, "Not found this command %s", command_type);
    }
    return RET_OK;
}

E_FAULT_CODE parserJson(const char *command)
{
    if( command == NULL)
    {
        return RET_NULL_POINTER;
    }

    cJSON * cj2 = cJSON_Parse(command);
    if(cj2 == NULL){
        Trace(1,"Get Json Fialed");
        return RET_NULL_POINTER;
    }
    cJSON* command_type = cJSON_GetObjectItem(cj2, MQTT_COMMAND_TYPE);
    cJSON* command_content = cJSON_GetObjectItem(cj2, MQTT_COMMAND_CONTENT);
    if(command_type == NULL || command_content == NULL)
    {
        cJSON_Delete(cj2);
        return RET_NULL_POINTER;
    }

    parserGPSCommand(command_type->valuestring, command_content->valuestring);

    cJSON_Delete(cj2);
    return RET_OK;

}


