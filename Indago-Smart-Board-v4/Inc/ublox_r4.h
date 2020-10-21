/********************************************************************************
  *
  * @file       : ublox_r4.h
  * @brief      : Header file for communicating with U-Blox R4xx Cellular Module via AT Commands
  * @author     : Charles H
  * @date       : July 2020
  *
*********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"
#include "ublox_gps.h"


// Defines
#define TX_BUFFER_SIZE      100
#define RX_BUFFER_SIZE      100
#define MSG_BUFFER_SIZE     5


typedef enum SARA_R4_Msg {
  AT_COMMAND_ECHO,
  AT_COMMAND_RESPONSE,
  AT_COMMAND_OK,
  AT_COMMAND_ERROR,
  AT_COMMAND_UNKNOWN
} SARA_R4_Msg_t;

typedef enum SARA_R4_Status {
  SARA_OK,
  SARA_TIMEOUT,
  SARA_ERROR, //TODO: List some of the other errors
} SARA_R4_Status_t;

typedef struct SARA_R4_Resp {
  uint8_t len;
  char message[40];
  SARA_R4_Msg_t msgType;
} SARA_R4_Resp_t;

typedef struct SARA_R4_Indication {  //TODO LABEL Values accordingly
  uint8_t battchg;
  uint8_t signal;
  uint8_t service;
  uint8_t sounder;
  uint8_t message;
  uint8_t call;
  uint8_t roam;
  uint8_t smsfull;
  uint8_t gprs;
  uint8_t callsetup;
  uint8_t callheld;
  uint8_t simind;
  TimeStamp_t timestamp;
} SARA_R4_Indication_t;

typedef struct SARA_R4_Operator {
  uint8_t mode;
  uint8_t format;
  char operator[24];
  uint8_t status;
  uint8_t accessTechnology;
} SARA_R4_Operator_t;

// Function Definitions
bool SARA_R4_Init(UART_HandleTypeDef *uart);
void SARA_R4_HW_Power_On(void);
void SARA_R4_HW_Reset(void);

/* UBLOX AT-COMMAND ACCESS FUNCTIONS */
SARA_R4_Status_t SARA_R4_Hex_Mode_Config(uint8_t mode);
SARA_R4_Status_t SARA_R4_Get_Manufacturer_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_Model_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_Firmware_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_IMEI(char* id);
SARA_R4_Status_t SARA_R4_Get_IMSI(char* id);
SARA_R4_Status_t SARA_R4_Get_ICCID(char* id);


/* UBLOX PHONE CONFIGURATION */
void SARA_R4_Shutdown(void);
void SARA_R4_Get_Functionality_Level(uint8_t *power);
void SARA_R4_Set_Functionality_Level(uint8_t fun, bool reset);
void SARA_R4_Get_Indications(SARA_R4_Indication_t *status);
void SARA_R4_Set_Event_Reporting(void);
void SARA_R4_Set_Timezone_Update(void);

/* UBLOX CELLULAR NETWORK REGISTRATION */
void SARA_R4_Get_Subscriber_Number(void);
void SARA_R4_Get_Signal_Quality(uint8_t *dbm);
void SARA_R4_Get_Extended_Signal_Quality(uint8_t *rxlev, uint8_t *ber, uint8_t *rscp, uint8_t *enc0, uint8_t *rsrq, uint8_t *rsrp);
void SARA_R4_Get_RAT(uint8_t *rat);
void SARA_R4_Get_Operator(SARA_R4_Operator_t *oper);
void SARA_R4_Set_Operator(SARA_R4_Operator_t *oper);
void SARA_R4_Get_Network_Registration(void);
void SARA_R4_Set_Network_Registration(void);
void SARA_R4_Get_Network_Selection(void);
void SARA_R4_Set_Network_Selection(void);
void SARA_R4_Get_Band_Selection(void);
void SARA_R4_Set_Band_Selection(void);

/* SIM MANAGEMENT */

/* INTERNET AND SECURITY CONFIGURATION */
void SARA_R4_CPRF_Set_TLS(uint8_t profile, uint8_t ssl_level, uint8_t cipher_suite, string root_cert, string client_cert, string client_key);
uint8_t SARA_R4_Create_Socket(uint8_t protocol, uint8_t *socket);
void SARA_R4_SSL_Mode_Config(uint8_t socket, uint8_t ssl_enable, uint8_t profile_id);
void SARA_R4_Setup_Socket(void);
void SARA_R4_Connect_Socket(uint8_t socket, char* addr, uint16_t port);
void SARA_R4_Write_Socket(uint8_t socket, uint8_t *data, uint16_t len);


/* FILE SYSTEM MANAGEMENT */
SARA_R4_Status_t SARA_R4_Clear_Files(void);
SARA_R4_Status_t SARA_R4_Download_File(char* f_name, char* data, uint16_t data_len);
SARA_R4_Status_t SARA_R4_Download_Block(char* f_name, char* data, uint16_t data_len);



/* LOW LEVEL UBLOX INTERFACE */
SARA_R4_Status_t SARA_R4_Send(char* msg);
SARA_R4_Status_t SARA_R4_SendN(char* msg, uint16_t len);
SARA_R4_Status_t SARA_R4_Receive(SARA_R4_Msg_t msgType, char *msg, uint16_t timeout);
void SARA_R4_IRQ_Handler(UART_HandleTypeDef *huart);