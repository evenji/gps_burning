#include "stdbool.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"
#include "config_mgr.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_fs.h"
#include "api_charset.h"

#include "cJSON.h"


#define TF_AUDIO_CONFIG_PATH "/t/audio/audio_config.json"
#define TF_AUDIO_PATH "/t/audio/"

#define TF_AUDIO_CONFIG_AUDIO_LISTS_TITLE "audio_lists"

AudioConfig_T g_audioConfig ;

E_FAULT_CODE parserAudioConfig(char* data, AudioConfig_T * audioconfig)
{
    cJSON *arr = NULL, *audio = NULL;
    int size = 0, idx = 0, i = 0;
    if( NULL == data || NULL == audioconfig)
    {
        return RET_NULL_POINTER;
    }
    Trace(1,"len = %djson = %s",strlen(data),data);
    cJSON * cj2 = cJSON_Parse(data);
    if(cj2 == NULL){
        Trace(1,"Get Json Fialed");
        return RET_NULL_POINTER;
    }


    arr = cJSON_GetObjectItem(cj2, TF_AUDIO_CONFIG_AUDIO_LISTS_TITLE);
    size = cJSON_GetArraySize(arr);
    for(idx=0; idx<size; idx++)
    {
        audio = cJSON_GetArrayItem(arr, idx);
        if( audio == NULL)
        {
            cJSON_Delete(cj2);
            return RET_NULL_POINTER;
        }
        cJSON * index   = cJSON_GetObjectItem( audio , "index") ;
        cJSON * title = cJSON_GetObjectItem( audio , "title") ;
        if(index == NULL || title == NULL)
        {
            cJSON_Delete(cj2);
            return RET_NULL_POINTER;
        }
        for(i = 0; i < AUDIO_MAX_NUMBER; i++)
        {
            if(i == atoi(index->valuestring))
            {
                audioconfig->audioInfo[i].index = atoi(index->valuestring);
                strncpy(audioconfig->audioInfo[i].AudioName, TF_AUDIO_PATH, AUDIO_NAME_MAX_LEN);
                strcat(audioconfig->audioInfo[i].AudioName, title->valuestring);
                audioconfig->audioInfo[i].vaild = 1;
            }
        }
    }
    cJSON_Delete(cj2);
    return RET_OK;
    
}

E_FAULT_CODE getAudioConfig(AudioConfig_T * audioconfig)
{
    int32_t fd;
    int32_t ret;
    uint8_t *path = TF_AUDIO_CONFIG_PATH;
    uint8_t readbuf[512] = {0};

    fd = API_FS_Open(path, (FS_O_RDONLY|FS_O_CREAT), 0);
	if ( fd < 0)
	{
        Trace(1,"Open file failed:%d",fd);
		return RET_OPEN_FILE_FAILED;
    }
    ret = API_FS_Read(fd, readbuf, sizeof(readbuf)) ;
    Trace(1,"read ret:%d",ret);
    API_FS_Close(fd);

    if(ret <= 0)
        return RET_READ_FILE_FAILED;

    return parserAudioConfig("{\"audio_lists\":[{\"index\":\"0\",\"title\":\"test1.mp3\"},{\"index\":\"1\",\"title\":\"test2.mp3\"},{\"index\":\"2\",\"title\":\"test3.mp3\"}]}}", audioconfig);
}