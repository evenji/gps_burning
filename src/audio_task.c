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
#include "config_mgr.h"

#define AUDIO_PLAY_TASK_STACK_SIZE    (2048 * 2)
#define AUDIO_PLAY_TASK_PRIORITY      4
#define AUDIO_PLAY_TASK_NAME          "AUDIO PLAY Task"

static bool playEnd = true;
HANDLE audioTaskHandle = NULL;

#define ALL_LOOP 65535
uint16_t playtimes = 0;
uint8_t currentIndex = 0;

void PlayCallback(AUDIO_Error_t result)
{
    Trace(1,"play callback result:%d",result);
    if(AUDIO_ERROR_END_OF_FILE == result)
        Trace(1,"play music file end");
    playEnd = true;
    Trace(1, "Audio: play end, playtimes = %d", playtimes);
    //AUDIO_Stop();
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
        Trace(1,"Audio: play music %s now",audioPath);
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
    }
    else
    {
        Trace(1, "audio is invaild");
    }
    return RET_OK;
    
}

void AudioTaskEventDispatch(Audio_Event_t* pEvent)
{
    switch(pEvent->id)
    {
        case AUDIO_EVENT_PLAY:
            Trace(1, "Audio: Get play audio command isLoop = %ld", pEvent->audioControl->isLoop);
            currentIndex = pEvent->audioControl->index;
            AUDIO_SpeakerSetVolume(pEvent->audioControl->volume);
            //playAudio(currentIndex);
            playtimes = pEvent->audioControl->isLoop;
            break;
        
        default:
            break;
    }
}

void AudioPlayTask(void *pData)
{
    while(1)
    {
        if(playEnd == true)
        {
            if(playtimes == ALL_LOOP)
            {
                playAudio(currentIndex);
            }
            else if(playtimes > 0  && playtimes != ALL_LOOP)
            {
                playAudio(currentIndex);
                playtimes--;
            }
            else if(playtimes == 0)
            {

            }
        }
        OS_Sleep(500); 
    }
}

void AudioTask(void *pData)
{
    Audio_Event_t* event=NULL;
    OS_Sleep(3000);    

    AUDIO_SetMode(AUDIO_MODE_LOUDSPEAKER);
    AUDIO_SpeakerOpen();
    AUDIO_SpeakerSetVolume(4);
    OS_Sleep(500);   

    E_FAULT_CODE ret = getAudioConfig(&g_audioConfig);
    if(ret == RET_OK)
    {
        OS_CreateTask(AudioPlayTask,
        NULL, NULL, AUDIO_PLAY_TASK_STACK_SIZE, AUDIO_PLAY_TASK_PRIORITY, 0, 0, AUDIO_PLAY_TASK_NAME);
    }
    else
    {
        Trace(1, "Audio: get audio config failed");
    }
    
    Trace(1, "Audio Task");
    while(1)
    {
        if(OS_WaitEvent(audioTaskHandle, (void**)&event, OS_TIME_OUT_WAIT_FOREVER))
        {
            AudioTaskEventDispatch(event);
            OS_Free(event);
        }
    }
}