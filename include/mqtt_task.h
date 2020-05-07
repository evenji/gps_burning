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
    MQTT_STATUS_NEED_RESTART,
    MQTT_STATUS_START_CONNECT,
    MQTT_STATUS_MAX
}MQTT_Status_t;

#define MQTT_TASK_STACK_SIZE    (2048 * 4)
#define MQTT_TASK_PRIORITY      2
#define MQTT_TASK_NAME          "MQTT Task"

#define EVEN


#ifdef EVEN
#define BROKER_IP  "9cexqiy.mqtt.iot.gz.baidubce.com"
#define BROKER_PORT 1883
#define CLIENT_ID  "Neucrack1"
#define CLIENT_USER "9cexqiy/client"
#define CLIENT_PASS "AnaRsIdLcOLh0TXs"
#else
#define BROKER_IP  "47.97.165.160"
#define BROKER_PORT 9210
#define CLIENT_ID  "Neucrack1"
#define CLIENT_USER "gpsTester"
#define CLIENT_PASS "system"
#endif

#define SUBSCRIBE_TOPIC "gps_command"
#define PUBLISH_TOPIC   "gps_info"
#define PUBLISH_INTERVAL_GPS 1000 //10s
#define PUBLISH_PAYLOEAD "{\"reported\":{\"version\":\"1.0\",\"serial_number\":\"20200119\",\"package_number\":0,\"date\":\"20200119 12:00:00\",\"longitude\":\"120.1\",\"latitude\":\"23.12\",\"altitude\":\"12\",\"speed\":\"10\",\"direction\":0,\"battery\":\"100\",\"fault_code\":0}}"

extern bool isStart;
extern HANDLE MqttTaskHandle;
extern HANDLE semMqttStart;
extern MQTT_Status_t mqttStatus;

void MqttTask(void *pData);

#endif