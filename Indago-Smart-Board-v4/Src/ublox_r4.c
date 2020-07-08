/********************************************************************************
  *
  * @file       : ublox_r4.c
  * @brief      : Source file for communicating with U-Blox R4xx Cellular Module via AT Commands
  * @author     : Charles H
  * @date       : July 2020
  *
*********************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "ublox_r4.h"
#include "queue.h"


/* Ublox Hardware Functions */
void R4_HW_Power_On(void){}

void R4_HW_Reset(void){}

/* Ublox Software Functions */
void R4_Init(void){}


// Ublox Phone Settings
void SARA_R4_Get_Manaufacturer_ID(void){}
void SARA_R4_Get_Model_ID(void){}
void SARA_R4_Get_Firmware_ID(void){}
void SARA_R4_Get_IMEI(void){}
void SARA_R4_Get_IMSI(void){}
void SARA_R4_Get_ICCID(void){}

// Ublox Phone Configuration
void SARA_R4_Shutdown(void){}

void SARA_R4_Get_Functionality_Level(void){}
void SARA_R4_Set_Functionality_Level(void){}

void SARA_R4_Get_Indications(void){}
void SARA_R4_Set_Event_Reporting(void){} //CMER?
void SARA_R4_Set_Timezone_Update(void){}

// UBlox Cellular Network Registration
void SARA_R4_Get_Subscriber_Number(void){}
void SARA_R4_Get_Signal_Quality(void){}
void SARA_R4_Get_Extended_Signal_Quality(void){}
void SARA_R4_Get_RAT(void){}
void SARA_R4_Get_RSRP(void){}
void SARA_R4_Get_RSRQ(void){}


void SARA_R4_Get_Operator(void){}
void SARA_R4_Set_Operator(void){}

void SARA_R4_Get_Network_Registration(void){}
void SARA_R4_Set_Network_Registration(void){}

void SARA_R4_Get_Network_Selection(void){}
void SARA_R4_Set_Network_Selection(void){}

void SARA_R4_Get_Band_Selection(void){}
void SARA_R4_Set_Band_Selection(void){}


// SIM Management



// Internet & Security


/* AT Parsing Functions */