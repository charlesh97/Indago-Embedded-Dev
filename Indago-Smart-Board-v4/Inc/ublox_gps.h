/********************************************************************************
  *
  * @file       : ublox_gps.h
  * @brief      : Header file for communicating with U-Blox GPS Receivers over UBX Protocol.
  * @author     : Charles H
  * @date       : July 2020
  *
*********************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx_hal.h"

typedef enum {
  UBX_I2C_PORT = 0,
  UBX_UART_PORT = 1,
  UBX_USB_PORT = 3,
  UBX_SPI_PORT = 4,
} UBX_Port_Id_t;

typedef struct UBX_Port_Config {
  uint8_t portId;
  uint16_t txReady;
  uint8_t mode;
  uint32_t baudRate;
  uint8_t inProtoMask;
  uint8_t outProtoMask;
  uint8_t flags;
} UBX_Port_Config_t;


// UBX Packet Header Definitions
#define UBX_SYNC_CHAR1          0xB5
#define UBX_SYNC_CHAR2          0x62

// Message Class and ID Definitions
#define UBX_CFG_PRT_CLASS       0x06
#define UBX_CFG_PRT_ID          0x00



// Function definitions
bool UBX_GPS_Init(UART_HandleTypeDef *uart);
void UBX_Get_Port_Config(UBX_Port_Id_t port);
void UBX_Set_Port_Config(UBX_Port_Config_t config);
void UBX_IRQ_Handler(UART_HandleTypeDef *huart);
