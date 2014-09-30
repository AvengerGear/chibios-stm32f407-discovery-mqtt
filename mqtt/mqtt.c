#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "MQTTClient.h"
#include "mqtt.h"
#include <string.h>

#if LWIP_NETCONN


void messageArrived(MessageData* md)
{
  MQTTMessage* message = md->message;

  chprintf((void *) &SD6, "%*.*s\r\n", (int) message->payloadlen, (int) message->payloadlen, (char *) message->payload);

  if (strncmp((char *) message->payload, "on", (int) message->payloadlen) == 0) {
    palSetPad(GPIOD, GPIOD_LED5);
    palSetPad(GPIOD, GPIOD_PIN0);
  } else if (strncmp((char *) message->payload, "off", (int) message->payloadlen) == 0) {
    palClearPad(GPIOD, GPIOD_LED5);
    palClearPad(GPIOD, GPIOD_PIN0);
  }
}

WORKING_AREA(wa_mqtt_client, MQTT_THREAD_STACK_SIZE);

msg_t mqtt_client(void *p) {
  int rc = 0;
  Network n;
  Client c;
  unsigned char buf[100];
  unsigned char readbuf[100];

  (void)p;

  /* Create a new TCP connection handle */
  chprintf((void *) &SD6, "Connecting to 192.168.1.1\r\n");
  NewNetwork (&n);
  rc = ConnectNetwork (&n, "192.168.1.1", 1883);
  if (rc != 0) {
    chprintf((void *) &SD6, "Failed to connect to server\r\n");
    goto blink_to_death;
  }

  chprintf((void *) &SD6, "Initializing MQTT service\r\n");
  MQTTClient (&c, &n, 1000, buf, 100, readbuf, 100);
  MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
  data.willFlag = 0;
  data.MQTTVersion = 3;
  data.clientID.cstring = "discovery";
  data.username.cstring = "discovery";
  data.password.cstring = "hello";
  /* data.keepAliveInterval = 30; */
  data.cleansession = 1;
  rc = MQTTConnect (&c, &data);
  if (rc == MQTT_FAILURE) {
    chprintf((void *) &SD6, "Failed to send connect request\r\n");
    goto blink_to_death;
  }

  chprintf((void *) &SD6, "Subscribing to its topic\r\n");
  rc = MQTTSubscribe(&c, "mqtt-subscription-discoveryqos0", QOS0, messageArrived);
  if (rc == MQTT_FAILURE) {
    chprintf((void *) &SD6, "Failed to subscribe to its topic\r\n");
    MQTTDisconnect(&c);
    goto blink_to_death;
  }
  chprintf((void *) &SD6, "Connected to 192.168.1.1\r\n");

  while(1) {
    MQTTYield(&c, 1000);
  }


  return RDY_OK;

blink_to_death:
  while(1){
    palClearPad(GPIOD, GPIOD_LED5);
    chThdSleepMilliseconds(50);
    palSetPad(GPIOD, GPIOD_LED5);
    chThdSleepMilliseconds(50);
  }
  return RDY_OK;
}

#endif /* LWIP_NETCONN */

/** @} */
