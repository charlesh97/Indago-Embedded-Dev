/********************************************************************************
  *
  * @file       : iot_cloud.c
  * @brief      : Interface for cloud communication. Integrates MQTT interface, JWT generation,
                  UBLOX Hardware Setup. Provides accessibility for all cloud functions. Currently setup
                  for Google Cloud.
  * @author     : Charles H
  * @date       : Feb 2021
  *
*********************************************************************************/

#include "iot_cloud.h"
#include "MQTTClient.h"
#include "ublox_r4.h"

#include <string.h>
#include <stdio.h>

SARA_R4_t *pModem;
Network *pNetwork;
MQTTClient *pClient;
char MQTT_TxBuf[MQTT_TX_BUF_SIZE];
char MQTT_RxBuf[MQTT_RX_BUF_SIZE];

void Cloud_Init(){

  free(pClient) //TODO: FREE NULL variable?
  pClient = (*MQTTClient)malloc(sizeof(MQTTClient));
  
  //? Fix this allocation
  Network network = {
    id = 0,
    mqttread = ubloxmqttread,
    mqttwrite = ubloxmqttwrite
  };

  //check return here
  MQTTClientInit(pClient, network, MQTT_CMD_TIMEOUT_MS, MQTT_TxBuf, MQTT_TX_BUF_SIZE, MQTT_RxBuf, MQTT_RX_BUF_SIZE);
}

void Cloud_Connect()

void Cloud_Send_GPS_Data(float lat, float lon){
  char payload[20];
  sprintf(payload, "{\"lat\":%f,\"lon\":%f}", lat, lon);
  MQTTMessage mqtt_msg;
  msg.qos = 1;
  msg.retained = 0; //?
  msg.dup = 0; //?
  msg.id = 0; //?
  msg.payload = payload;
  msg.payload = strlen(payload);

  MQTTPublish(client, &mqtt_msg);
}


/********** Functions for MQTTClient-C **********/
TimerInit(Timer*){

}
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);

int ubloxmqttread(Network*, unsigned char* read_buffer, int, int){

}

//Network, Buffer, Len, TimerMS
int ubloxmqttwrite(Network* network, unsigned char* send_buffer, int len, int remainingMS){
  char send[200];
  sprintf(send, "AT+USOWR=%d,%d\"%s\"\r\n", modem->socket.id, len, send_buffer);
  if(SARA_R4_Send(send_buffer) != AT_OK)
    return -1;
  else
    return 1;
}