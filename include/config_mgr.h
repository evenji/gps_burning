#ifndef _CONFIG_MGR_H_
#define _CONFIG_MGR_H_
#include "fault_code.h"


#define AUDIO_MAX_NUMBER 32
#define AUDIO_NAME_MAX_LEN 128
typedef struct AudioInfo_Tag{
    uint8_t index;
    char AudioName[AUDIO_NAME_MAX_LEN];
    uint8_t vaild;
}AudioInfo_T;
typedef struct AudioConfig_Tag{
    AudioInfo_T audioInfo[AUDIO_MAX_NUMBER];
}AudioConfig_T;

extern AudioConfig_T g_audioConfig ;
E_FAULT_CODE getAudioConfig(AudioConfig_T * audioconfig);

#endif