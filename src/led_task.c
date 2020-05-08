#include "stdint.h"
#include "stdbool.h"
#include "api_os.h"
#include "api_event.h"
#include "api_debug.h"
#include "api_mqtt.h"
#include "api_network.h"
#include "api_socket.h"
#include "api_key.h"
#include "gps.h"
#include "api_hal_gpio.h"
#include "api_hal_pm.h"

#include "led_task.h"

#define LED_ON  GPIO_LEVEL_HIGH
#define LED_OFF GPIO_LEVEL_LOW

#define LED_INV(led_status) led_status = (led_status==GPIO_LEVEL_HIGH)?GPIO_LEVEL_LOW:GPIO_LEVEL_HIGH;

typedef enum{
    LED_ALWAYS_ON_CNT = 0,
    LED_FAST_CNT      = 1,
    LED_LOW_CNT       = 3,
    LED_ALWAYS_OFF_CNT= 4
}LED_ON_CNT;

#define LED_ON_INIT_TIME 10000
#define LED_ON_TIME 10000

HANDLE LedTaskHandle = NULL;
extern HANDLE mainTaskHandle;

typedef enum{
    CMD_LED_ON = 1,
    CMD_LED_OFF = 2
}CMD_LED;

CMD_LED Led_Control = CMD_LED_OFF;

typedef struct ITME_QUALITY_Tag{
    int16_t always;
    int16_t fast;
    int16_t current
}ITME_QUALITY;

typedef struct LED_ITME_Tag
{
    GPIO_config_t led_cfg;
    GPIO_LEVEL    led_level;
    LED_ON_CNT    flicker_cnt;
    uint16_t      tick;
    ITME_QUALITY  quality
}LED_ITME;

LED_ITME LED_BAT = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN26,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_ALWAYS_ON_CNT,
    .tick = 0,
    .quality = {
        .always = 80,
        .fast   = 60,
        .current= 0
    }
};

LED_ITME LED_GPRS = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN27,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_FAST_CNT,
    .tick = 0,
    .quality = {
        .always = 20,
        .fast   = 10,
        .current= 0
    }
};

LED_ITME LED_GPS = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN28,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_LOW_CNT,
    .tick = 0,
    .quality = {
        .always = 8,
        .fast   = 5,
        .current= 0
    }
};

GPIO_config_t PowerKeyInput = {
    .mode               = GPIO_MODE_INPUT,
    .pin                = GPIO_PIN29,
    .defaultLevel       = GPIO_LEVEL_HIGH,
    .intConfig.debounce = 0,
    .intConfig.type     = GPIO_INT_TYPE_MAX,
    .intConfig.callback = NULL
};



void LedInit()
{
    
    PM_PowerEnable(POWER_TYPE_VPAD, true);

    GPIO_Init(LED_BAT.led_cfg);
    GPIO_Init(LED_GPRS.led_cfg);
    GPIO_Init(LED_GPS.led_cfg);
    GPIO_Init(PowerKeyInput);    
}

void ItmeQualityHandle(LED_ITME *led)
{
    if(led->quality.current >= led->quality.always)
    {
        led->flicker_cnt = LED_ALWAYS_ON_CNT;
    }
    else if(led->quality.current >= led->quality.fast)
    {
        led->flicker_cnt = LED_FAST_CNT;
    }
    else 
    {
        led->flicker_cnt = LED_LOW_CNT;
    }
}

void LedControl(LED_ITME *led)
{
    if(CMD_LED_ON == Led_Control)
    {
        //judge level
        ItmeQualityHandle(led);

        if( led->flicker_cnt == LED_ALWAYS_ON_CNT)
        {
            led->led_level = LED_ON;
        }
        else if(led->flicker_cnt == LED_ALWAYS_OFF_CNT)
        {
            led->led_level = LED_OFF;
        }
        else if( led->tick >= led->flicker_cnt)
        {
            LED_INV(led->led_level);
            led->tick = 0;
        }
    }
    else if(CMD_LED_OFF == Led_Control)
    {
        led->led_level = GPIO_LEVEL_LOW;
    }

    GPIO_SetLevel(led->led_cfg,led->led_level);
}

void OnTimer(void* param)
{
    Trace(1, "Timer up!");
    Led_Control = CMD_LED_OFF;
}

void KeyEnableHandle()
{
    GPIO_LEVEL status=0;
    GPIO_GetLevel(PowerKeyInput,&status);
    if( status == GPIO_LEVEL_LOW)
    {
        if(CMD_LED_OFF == Led_Control )
        {
            Led_Control = CMD_LED_ON;
            OS_StartCallbackTimer(mainTaskHandle,LED_ON_TIME,OnTimer,NULL);

        }
        else if(CMD_LED_ON == Led_Control)
        {

        }
    }
    
}

void LedTask(void *pData)
{
    LedInit();

    Led_Control = CMD_LED_ON;
    OS_StartCallbackTimer(mainTaskHandle,LED_ON_INIT_TIME,OnTimer,NULL);

    Trace(1, "Led Task");
    while(1)
    {

        LedControl(&LED_BAT);
        LedControl(&LED_GPRS);
        LedControl(&LED_GPS);

        KeyEnableHandle();

        LED_BAT.tick++;
        LED_GPRS.tick++;
        LED_GPS.tick++;
        OS_Sleep(300);
    }
}

void LED_Set_Bat_Quality(int16_t current)
{
    Trace(1, "Battery Quality: %d", current);
    LED_BAT.quality.current = current;
}

void LED_Set_GPRS_Quality(int16_t current)
{
    Trace(1, "GPRS Quality: %d", current);
    LED_GPRS.quality.current = current;
}

void LED_Set_GPS_Quality(int16_t current)
{
    Trace(1, "GPS Quality: %d", current);
    LED_GPS.quality.current = current;
}