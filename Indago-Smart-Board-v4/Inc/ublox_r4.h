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


// Defines
#define TX_BUFFER_SIZE      100
#define RX_BUFFER_SIZE      100
#define MSG_BUFFER_SIZE     5


typedef enum SARA_R4_Msg_Type{
  AT_COMMAND_ECHO,
  AT_COMMAND_RESPONSE,
  AT_COMMAND_OK,
  AT_COMMAND_ERROR,
  AT_COMMAND_UNKNOWN
} SARA_R4_Msg_Type_t;


typedef struct SARA_R4_Resp{
  uint8_t len;
  char message[40];
  SARA_R4_Msg_Type_t msgType;
} SARA_R4_Resp_t;

typedef enum SARA_R4_Status{
  SARA_OK,
  SARA_ERROR, //TODO: List some of the other errors
} SARA_R4_Status_t;

// Function Definitions
bool SARA_R4_Init(UART_HandleTypeDef *uart);
void SARA_R4_HW_Power_On(void);
void SARA_R4_HW_Reset(void);
void SARA_R4_IRQ_Handler(UART_HandleTypeDef *huart);


SARA_R4_Status_t SARA_R4_Get_Manaufacturer_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_Model_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_Firmware_ID(char* id);
SARA_R4_Status_t SARA_R4_Get_IMEI(char* id);
SARA_R4_Status_t SARA_R4_Get_IMSI(char* id);
SARA_R4_Status_t SARA_R4_Get_ICCID(char* id);