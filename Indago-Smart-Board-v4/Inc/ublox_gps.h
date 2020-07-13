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
  UBX_Port_Id_t portId;
  uint16_t txReady;
  uint16_t mode;
  uint32_t baudRate;
  uint16_t inProtoMask;
  uint16_t outProtoMask;
  uint8_t flags;
} UBX_Port_Config_t;

typedef struct UBX_Config_Storage {
  uint32_t clearMask;
  uint32_t saveMask;
  uint32_t loadMask;
  uint8_t deviceMask; // This is optional
} UBX_Config_Storage_t;

typedef struct TimeStamp { // TODO THIS NEEDS TO BE REPLACED WITH A REAL TIMESTAMP STRUCT
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} TimeStamp_t;


// UBX Packet Header Definitions
#define UBX_SYNC_CHAR1          0xB5
#define UBX_SYNC_CHAR2          0x62

// Message Class and ID Definitions
#define UBX_CFG_PRT_CLASS       0x06
#define UBX_CFG_PRT_ID          0x00

#define UBX_CFG_PRT_MODE_7BIT           (0x2 << 6)
#define UBX_CFG_PRT_MODE_8BIT           (0x3 << 6)
#define UBX_CFG_PRT_MODE_PARITY_EVEN    (0x0 << 9)
#define UBX_CFG_PRT_MODE_PARITY_ODD     (0x1 << 9)
#define UBX_CFG_PRT_MODE_PARITY_NONE    (0x4 << 9)
#define UBX_CFG_PRT_MODE_STOP_1         (0x0 << 12)
#define UBX_CFG_PRT_MODE_STOP_1_5       (0x1 << 12)
#define UBX_CFG_PRT_MODE_STOP_2         (0x2 << 12)
#define UBX_CFG_PRT_MODE_STOP_0_5       (0x3 << 12)

#define UBX_CFG_PRT_PROTO_UBX   0x01
#define UBX_CFG_PRT_PROTO_NMEA  0x02
#define UBX_CFG_PRT_PROTO_RTCM2 0x04
#define UBX_CFG_PRT_PROTO_RTCM3 0x20

#define UBX_CFG_PRT_FLAG_TXTIMEOUT      0x2




#define UBX_CFG_CFG_CLASS       0x06
#define UBX_CFG_CFG_ID          0x09

#define UBX_CFG_CFG_MASK_IOPORT       (0x1 << 0)
#define UBX_CFG_CFG_MASK_MSGCONF      (0x1 << 1)
#define UBX_CFG_CFG_MASK_INFMSG       (0x1 << 2)
#define UBX_CFG_CFG_MASK_NAVCONF      (0x1 << 3)
#define UBX_CFG_CFG_MASK_RXMCONF      (0x1 << 4)
#define UBX_CFG_CFG_MASK_SENCONF      (0x1 << 8)
#define UBX_CFG_CFG_MASK_RINVCONF     (0x1 << 9)
#define UBX_CFG_CFG_MASK_ANTCONF      (0x1 << 10)
#define UBX_CFG_CFG_MASK_LOGCONF      (0x1 << 11)
#define UBX_CFG_CFG_MASK_FTSCONF      (0x1 << 12)

#define UBX_CFG_CFG_DEVICE_BBR          0x01 // Battery backed RAM
#define UBX_CFG_CFG_DEVICE_FLASH        0x02
#define UBX_CFG_CFG_DEVICE_EEPROM       0x04
#define UBX_CFG_CFG_DEVICE_SPIFLASH     0x10

#define UBX_CFG_MSG_CLASS               0x06
#define UBX_CFG_MSG_ID                  0x01


#define UBX_NAV_PVT_CLASS               0x01
#define UBX_NAV_PVT_ID                  0x07


// Function definitions
bool UBX_GPS_Init(UART_HandleTypeDef *uart);
void UBX_Get_Port_Config(UBX_Port_Id_t port);
void UBX_Set_Port_Config(UBX_Port_Config_t config);

void UBX_Set_Message_Rate(uint8_t msg_class, uint8_t msg_id, uint8_t rate);
void UBX_Get_Message_Rate(uint8_t msg_class, uint8_t msg_id);

void UBX_Manage_Config(UBX_Config_Storage_t config);


void UBX_IRQ_Handler(UART_HandleTypeDef *huart);