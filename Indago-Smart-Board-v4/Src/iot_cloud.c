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



void Cloud_Init(){
  
}





/********** Timer Functions for MQTTClient-C **********/
TimerInit(Timer*){

}
char TimerIsExpired(Timer*);
void TimerCountdownMS(Timer*, unsigned int);
void TimerCountdown(Timer*, unsigned int);
int TimerLeftMS(Timer*);