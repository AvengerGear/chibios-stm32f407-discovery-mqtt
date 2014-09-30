#ifndef _MQTT_H_
#define _MQTT_H_

#ifndef MQTT_THREAD_STACK_SIZE
#define MQTT_THREAD_STACK_SIZE   1024
#endif

#ifndef MQTT_THREAD_PORT
#define MQTT_THREAD_PORT         80
#endif

#ifndef MQTT_THREAD_PRIORITY
#define MQTT_THREAD_PRIORITY     (LOWPRIO + 2)
#endif

extern WORKING_AREA(wa_mqtt_client, MQTT_THREAD_STACK_SIZE);
msg_t mqtt_client(void *p);

#endif /* _MQTT_H_ */
