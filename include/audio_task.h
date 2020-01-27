#ifndef _AUDIO_TASK_H_
#define _AUDIO_TASK_H_

typedef enum{
    AUDIO_EVENT_PLAY = 0,
    AUDIO_EVENT_MAX
}Audio_Event_ID_t;

typedef struct AudioControl_Tag{
    uint8_t index;
    uint8_t volume;
    uint16_t isLoop;
}AudioControl_T;

typedef struct {
    Audio_Event_ID_t id;
    AudioControl_T* audioControl;
}Audio_Event_t;

#define AUDIO_TASK_STACK_SIZE    (2048 * 2)
#define AUDIO_TASK_PRIORITY      3
#define AUDIO_TASK_NAME          "AUDIO Task"

void AudioTaskEventDispatch(Audio_Event_t* pEvent);
void AudioTask(void *pData);

extern HANDLE audioTaskHandle;

#endif