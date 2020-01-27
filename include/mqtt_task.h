#ifndef _MQTT_TASK_H_
#define _MQTT_TASK_H_

typedef enum{
    MQTT_EVENT_CONNECTED = 0,
    MQTT_EVENT_DISCONNECTED ,
    MQTT_EVENT_MAX
}MQTT_Event_ID_t;


typedef struct {
    MQTT_Event_ID_t id;
    MQTT_Client_t* client;
}MQTT_Event_t;

typedef enum{
    MQTT_STATUS_DISCONNECTED = 0,
    MQTT_STATUS_CONNECTED       ,
    MQTT_STATUS_MAX
}MQTT_Status_t;

#define MQTT_TASK_STACK_SIZE    (2048 * 2)
#define MQTT_TASK_PRIORITY      2
#define MQTT_TASK_NAME          "MQTT Task"

#define BROKER_IP  "k5nahpt.mqtt.iot.gz.baidubce.com"
#define BROKER_PORT 1883
#define CLIENT_ID  "Neucrack1"
#define CLIENT_USER "k5nahpt/gps_monitor_001"
#define CLIENT_PASS "tak77yr89actiepn"
#define SUBSCRIBE_TOPIC "$neucrack/app"
#define PUBLISH_TOPIC   "$baidu/iot/shadow/gps_monitor_001/update"
#define PUBLISH_INTERVAL_GPS 1000 //10s
#define PUBLISH_PAYLOEAD "{\"reported\":{\"version\":\"1.0\",\"serial_number\":\"20200119\",\"package_number\":0,\"date\":\"20200119 12:00:00\",\"longitude\":\"120.1\",\"latitude\":\"23.12\",\"altitude\":\"12\",\"speed\":\"10\",\"direction\":0,\"battery\":\"100\",\"fault_code\":0}}"

extern HANDLE MqttTaskHandle;
extern HANDLE semMqttStart;

void MqttTask(void *pData);

#endif