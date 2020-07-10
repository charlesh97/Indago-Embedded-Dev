/********************************************************************************
  *
  * @file       : ublox_gps.c
  * @brief      : Source file for communicating with U-Blox GPS Receivers over UBX Protocol.
  * @author     : Charles H
  * @date       : July 2020
  *
*********************************************************************************/
#include <stdlib.h>
#include <string.h>

#include "ublox_gps.h"
#include "queue.h"


// Globals
static UART_HandleTypeDef *huart;
static Queue_t uart_tx_queue;


// Static Functions
static bool UBX_Send_Packet(uint8_t msg_class, uint8_t id, uint8_t *payload, uint16_t len);
static uint16_t UBX_Calculate_Checksum(uint8_t *payload, uint16_t len);


// Init
bool UBX_GPS_Init(UART_HandleTypeDef *uart){
  huart = uart;

  if(Queue_Init(&uart_tx_queue, sizeof(uint8_t), 100) != QUEUE_OK)
    return false;

  SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);                     // Clear RX ISR flags
  SET_BIT(huart->Instance->ICR, 0xFFFF);                                        // Clear all flags
  SET_BIT(huart->Instance->CR3, USART_CR3_EIE);                                 // Turn on RNE IRQ (Main UART IRQ)
  SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);                              // CR1|RE&UE already set from HAL_UART_Init()


  // Change the port config over to UBX
  UBX_Port_Config_t portConfig = {
    .portId = UBX_UART_PORT,
    .txReady = 0,
    .mode = UBX_CFG_PRT_MODE_8BIT |
            UBX_CFG_PRT_MODE_PARITY_NONE |
            UBX_CFG_PRT_MODE_STOP_1,
    .baudRate = 9600,
    .inProtoMask = UBX_CFG_PRT_PROTO_UBX,
    .outProtoMask = UBX_CFG_PRT_PROTO_UBX,
    .flags = 0,
  };
  UBX_Set_Port_Config(portConfig);

  HAL_Delay(500);
  UBX_Set_Message_Rate(UBX_NAV_PVT_CLASS, UBX_NAV_PVT_ID, 1);

  return true;
}

// Configuration
void UBX_Get_Port_Config(UBX_Port_Id_t port){
  UBX_Send_Packet(UBX_CFG_PRT_CLASS, UBX_CFG_PRT_ID, (uint8_t*)(&port), 1);
}

void UBX_Set_Port_Config(UBX_Port_Config_t config){
  uint8_t payload[20];
  payload[0] = config.portId;
  memcpy(payload + 2, &config.txReady, 2);
  memcpy(payload + 4, &config.mode, 2);
  payload[6] = 0;
  payload[7] = 0;
  memcpy(payload + 8, &config.baudRate, 4);
  memcpy(payload + 12, &config.inProtoMask, 2);
  memcpy(payload + 14, &config.outProtoMask, 2);
  memcpy(payload + 16, &config.flags, 2);

  UBX_Send_Packet(UBX_CFG_PRT_CLASS, UBX_CFG_PRT_ID, payload, 20);
}

// Allows you to clear, save, load configs
void UBX_Manage_Config(UBX_Config_Storage_t config){
  uint8_t payload[13];
  memcpy(payload, &config.clearMask, 4);
  memcpy(payload, &config.saveMask, 4);
  memcpy(payload, &config.loadMask, 4);

  uint8_t len = 12;
  if(config.deviceMask){
    len = 13;
    payload[12] = config.deviceMask;
  }

  UBX_Send_Packet(UBX_CFG_CFG_CLASS, UBX_CFG_CFG_ID, payload, len);
}

void UBX_Set_Message_Rate(uint8_t msg_class, uint8_t msg_id, uint8_t rate){
  uint8_t payload[3];
  payload[0] = msg_class;
  payload[1] = msg_id;
  payload[2] = rate;
  UBX_Send_Packet(UBX_CFG_MSG_CLASS, UBX_CFG_MSG_ID, payload, 3);
}

void UBX_Get_Message_Rate(uint8_t msg_class, uint8_t msg_id){
  uint8_t payload[2] = {msg_class, msg_id};
  UBX_Send_Packet(UBX_CFG_MSG_CLASS, UBX_CFG_MSG_ID, payload, 2);
}

// Receive data streaming


// Low level functions
static bool UBX_Send_Packet(uint8_t msg_class, uint8_t id, uint8_t *payload, uint16_t len){
  uint8_t* packet = (uint8_t*)malloc((8 + len)*sizeof(uint8_t));

  packet[0] = UBX_SYNC_CHAR1;
  packet[1] = UBX_SYNC_CHAR2;
  packet[2] = msg_class;
  packet[3] = id;
  packet[4] = len & 0x00FF;
  packet[5] = (len & 0xFF00) >> 8;

  memcpy(packet + 6, payload, len);
  uint16_t chksum = UBX_Calculate_Checksum(packet+2, len+4);
  packet[6 + len] = (chksum & 0xFF00) >> 8;     // CK_A
  packet[7 + len] = (chksum & 0x00FF);          // CK_B

  bool ret = Queue_IsFull(&uart_tx_queue);
  if(!ret)
  {
    for (uint16_t i = 1; i < 8+len; i++){
      if(Queue_Append(&uart_tx_queue, packet + i) == QUEUE_FULL){
        free(packet);
        return false;
      }
    }
    huart->Instance->TDR = packet[0];
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn on TX interrupts
  }
  free(packet);
  return ret;
}

// Checksum calculations
static uint16_t UBX_Calculate_Checksum(uint8_t *payload, uint16_t len){
  uint8_t ck_a = 0;
  uint8_t ck_b = 0;

  uint8_t i;
  for(i = 0; i < len; i++){
    ck_a = ck_a + payload[i];
    ck_b = ck_b + ck_a;
  }

  return (ck_a << 8) | ck_b;
}

void UBX_IRQ_Handler(UART_HandleTypeDef *huart)
{
  // Check for error interrupt and handle appropriately - Pulled from hal_uart.c
  uint32_t isrflags = huart->Instance->ISR;
  uint32_t errorflags = (isrflags & (uint32_t)(USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE | USART_ISR_RTOF));

  // Handle these errors accordingly and throw away the whole message
  if (errorflags != 0x00)
  {
    SET_BIT(huart->Instance->ICR, 0xFFFF);              // Clear error ISR flags
    return;
  }

  // RX UART ISR
  if (isrflags & USART_ISR_RXNE)
  {
    uint8_t data = huart->Instance->RDR; // Read out data
    data++;
  }

  // TX UART ISR (TXEIE must be enabled)
  if (isrflags & USART_ISR_TXE && huart->Instance->CR1 & USART_CR1_TXEIE)
  {
    uint8_t tx;
    if (Queue_Get(&uart_tx_queue, &tx) == QUEUE_OK) // Push new data
      huart->Instance->TDR = tx;
    else{
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn off TX interrupts
    }
  }
}