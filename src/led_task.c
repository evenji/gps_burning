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

#define LED_ALWAYS_ON_CNT 0
#define LED_ALWAYS_OFF_CNT 255

typedef struct LED_ITME_Tag
{
    GPIO_config_t led_cfg;
    GPIO_LEVEL    led_level;
    uint16_t      flicker_cnt;
    uint16_t      tick
}LED_ITME;

LED_ITME LED_BAT = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN26,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_ALWAYS_ON_CNT,
    .tick = 0
};

LED_ITME LED_GPRS = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN27,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_ALWAYS_ON_CNT,
    .tick = 0
};

LED_ITME LED_GPS = {
    .led_cfg = {
        .mode         = GPIO_MODE_OUTPUT,
        .pin          = GPIO_PIN28,
        .defaultLevel = LED_OFF
    },
    .led_level = LED_OFF,
    .flicker_cnt = LED_ALWAYS_ON_CNT,
    .tick = 0
};



void LedInit()
{
    
    PM_PowerEnable(POWER_TYPE_VPAD, true);

    GPIO_Init(LED_BAT.led_cfg);
    GPIO_Init(LED_GPRS.led_cfg);
    GPIO_Init(LED_GPS.led_cfg);    
}

void LedControl(LED_ITME *led)
{
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
    GPIO_SetLevel(led->led_cfg,led->led_level);
}

void LedTask(void *pData)
{
    LedInit();

    Trace(1, "Led Task");
    while(1)
    {

        LedControl(&LED_BAT);
        LedControl(&LED_GPRS);
        LedControl(&LED_GPS);

        LED_BAT.tick++;
        LED_GPRS.tick++;
        LED_GPS.tick++;
        OS_Sleep(300);
    }
}