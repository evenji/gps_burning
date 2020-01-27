#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "string.h"

#include "api_os.h"
#include "api_debug.h"
#include "api_event.h"
#include "api_audio.h"
#include "api_fs.h"
#include "config_mgr.h"
#include "audio_task.h"
#include "fault_code.h"


static bool playEnd = true;

void PlayCallback(AUDIO_Error_t result)
{
    Trace(1,"play callback result:%d",result);
    if(AUDIO_ERROR_END_OF_FILE == result)
        Trace(1,"play music file end");
    playEnd = true;
    AUDIO_Stop();
}

E_FAULT_CODE playAudio(uint8_t index)
{
    if(index >= AUDIO_MAX_NUMBER)
    {
        Trace(1, "input index is out of range : %d", index);
        return RET_INDEX_OUT_OF_RANGE;
    }
    if( g_audioConfig.audioInfo[index].vaild == 1)
    {
        char * audioPath = &(g_audioConfig.audioInfo[index].AudioName[0]);
        if(audioPath == NULL)
        {
            return RET_NULL_POINTER;
        }
        Trace(1,"play music %s now",audioPath);
        AUDIO_Error_t ret = AUDIO_Play(audioPath,AUDIO_TYPE_MP3,PlayCallback);
        if( ret != AUDIO_ERROR_NO)
        {
            Trace(1,"play audio fail:%d",ret);
            return RET_PLAY_AUDIO_FAILED;
        }
        else
        {
            playEnd = false;
            Trace(1,"play music success");
        }
        OS_Sleep(200);
        AUDIO_SetMode(AUDIO_MODE_LOUDSPEAKER);
        AUDIO_SpeakerOpen();
        AUDIO_SpeakerSetVolume(4);
    }
    else
    {
        Trace(1, "audio is invaild");
    }
    
}

void AudioTask(void *pData)
{
    OS_Sleep(3000);
    Trace(1, "Audio Task");

    while(1)
    {
        OS_Sleep(1000);
    }
}