#include "agent.h"
#include "gps_task.h"
#include "sys_config.h"
#include "api_debug.h"
#include "cJSON.h"

#define MQTT_COMMAND_TYPE "command_type"
#define MQTT_COMMAND_TYPE_PLAY_AUDIO "play_audio"

#define MQTT_COMMAND_CONTENT "command_content"

typedef struct AudioControl_Tag{
    uint8_t index;
    uint8_t volume;
    uint8_t isLoop;
}AudioControl_T;

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
    char speed[8];
    int direction;
    char battery[8];
    uint32_t fault_code;
}DevInfo_T;

char* devInfoJson = "{\"reported\":{\"version\":\"%s\",\"serial_number\":\"%s\",\"package_number\":%d,\"date\":\"%s\",\"longitude\":\"%s\",\"latitude\":\"%s\",\"altitude\":\"%s\",\"speed\":\"%s\",\"direction\":%d,\"battery\":\"%s\",\"fault_code\":%d}}";

uint8_t buffer[300];
char * createDevInfoJsonStr(DevInfo_T *info)
{    
    snprintf(buffer, sizeof(buffer), devInfoJson, info->version, 
                info->sn, info->package_number, info->date,info->longitude,info->latitude,
                info->altitude, info->speed, info->direction,info->battery, info->fault_code);
    return &buffer[0];
}

char* getDevInfoJsonStr()
{
    DevInfo_T devinfo;
    strcpy(devinfo.version, SOFTWARE_VERSION);
    strcpy(devinfo.sn, SERIAL_NUMBER);
    devinfo.package_number = package_number;
    getGPSDate(devinfo.date);
    getGPSInfo(devinfo.longitude, devinfo.latitude, devinfo.altitude, devinfo.speed, &(devinfo.direction));
    strcpy(devinfo.battery, "100");
    devinfo.fault_code = 0;
    createDevInfoJsonStr(&devinfo);
    package_number++;
    return &buffer[0];
}

E_FAULT_CODE getAudioControl(char *command_content)
{
    AudioControl_T audioControl;
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

    audioControl.index = atoi(index->valuestring);
    audioControl.volume = atoi(volume->valuestring);
    audioControl.isLoop = atoi(isLoop->valuestring);

    Trace(1, "Get Audio Control Success, index = %d, volume = %d, isLoop = %d", audioControl.index, audioControl.volume, audioControl.isLoop);
  
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


