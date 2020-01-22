#include "agent.h"
#include "gps_task.h"
#include "sys_config.h"

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



    // cJSON * root =  cJSON_CreateObject();
    // cJSON * item =  cJSON_CreateObject();

    // cJSON_AddItemToObject(root, "reported", item);//root节点下添加reported节点
    // cJSON_AddItemToObject(item, "version", cJSON_CreateString("1.0"));
    // cJSON_AddItemToObject(item, "serial_number", cJSON_CreateString("20200119"));
    // cJSON_AddItemToObject(item, "package_number", cJSON_CreateNumber(0));
    // cJSON_AddItemToObject(item, "date", cJSON_CreateString("20200119 12:00:00"));
    // cJSON_AddItemToObject(item, "longitude", cJSON_CreateString("120.1"));
    // cJSON_AddItemToObject(item, "latitude", cJSON_CreateString("23.12"));
    // cJSON_AddItemToObject(item, "altitude", cJSON_CreateString("12"));
    // cJSON_AddItemToObject(item, "speed", cJSON_CreateString("10"));
    // cJSON_AddItemToObject(item, "direction", cJSON_CreateNumber(0));
    // cJSON_AddItemToObject(item, "battery", cJSON_CreateString("100"));
    // cJSON_AddItemToObject(item, "fault_code", cJSON_CreateNumber(0));

    // return cJSON_Print(root);