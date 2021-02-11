 /********************************************************************************
  *
  * @file       : iot_cloud.h
  * @brief      : Interface for cloud communication. Integrates MQTT interface, JWT generation,
                  UBLOX Hardware Setup. Provides accessibility for all cloud functions. Currently setup
                  for Google Cloud.
  * @author     : Charles H
  * @date       : Feb 2021
  *
*********************************************************************************/
#ifndef IOT_CLOUD_H
#define IOT_CLOUD_H




typedef struct Network
{
	int (*mqttread)(Network*, unsigned char* read_buffer, int, int);
	int (*mqttwrite)(Network*, unsigned char* send_buffer, int, int);
} Network;

typedef struct Timer
{
    
} Timer;


int mqttread(Network*, unsigned char* read_buffer, int, int);
int mqttwrite(Network*, unsigned char* send_buffer, int, int);

#endif //IOT_CLOUD_H