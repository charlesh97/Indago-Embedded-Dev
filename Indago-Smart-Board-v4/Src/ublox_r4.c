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
#include "main.h"

// Globals
static UART_HandleTypeDef *huart;
Queue_t cell_tx_queue, cell_rx_queue;
Queue_t message_queue;

SARA_R4_Resp_t tempMsg;

char testing[30];

bool SARA_R4_Send(char* msg);
SARA_R4_Status_t SARA_R4_Receive(char* msg, bool checkOK, uint16_t timeout);

/** MAIN UBLOX FUNCTIONS **/

//Init
bool SARA_R4_Init(UART_HandleTypeDef *uart){
  huart = uart;

  // Initialize Queues
  if(Queue_Init(&cell_tx_queue, sizeof(uint8_t), TX_BUFFER_SIZE) != QUEUE_OK)
    return false;
  if(Queue_Init(&cell_tx_queue, sizeof(uint8_t), RX_BUFFER_SIZE) != QUEUE_OK)
    return false;
  if(Queue_Init(&message_queue, sizeof(SARA_R4_Resp_t), MSG_BUFFER_SIZE) != QUEUE_OK)
    return false;

  // Power On
  SARA_R4_HW_Power_On();
  HAL_Delay(4000);

  // Initialize UART HW //TODO MOVE THIS STATEMENT
  SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);                     // Clear RX ISR flags
  SET_BIT(huart->Instance->ICR, 0xFFFF);                                        // Clear all flags
  SET_BIT(huart->Instance->CR3, USART_CR3_EIE);                                 // Turn on RNE IRQ (Main UART IRQ)
  SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);


  SARA_R4_Get_Manaufacturer_ID(testing);
  SARA_R4_Get_Model_ID(testing);
  SARA_R4_Get_Firmware_ID(testing);
  SARA_R4_Get_IMEI(testing);
  SARA_R4_Get_IMSI(testing);
  SARA_R4_Get_ICCID(testing);

  return true;
}

//Message Processing
//This is a non-blocking process that will process messages at commands
//and act on them accordingly??
void SARA_R4_Process_Messages(void){
}

/* Module HW Control */
void SARA_R4_HW_Power_On(void){
  HAL_GPIO_WritePin(GPIOA, CELL_ON_Pin, GPIO_PIN_RESET);
  HAL_Delay(1000);
  HAL_GPIO_WritePin(GPIOA, CELL_ON_Pin, GPIO_PIN_SET);
}

void SARA_R4_HW_Reset(void){
  HAL_GPIO_WritePin(GPIOA, CELL_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(500);
  HAL_GPIO_WritePin(GPIOA, CELL_RESET_Pin, GPIO_PIN_SET);
}



/* UBLOX AT-COMMAND ACCESS FUNCTIONS */

// Ublox Phone Settings
SARA_R4_Status_t SARA_R4_Get_Manaufacturer_ID(char* id){
  if(!SARA_R4_Send("at+cgmi\r\n"))
    return SARA_ERROR;
  return SARA_R4_Receive(id, true, 1000);
}
SARA_R4_Status_t SARA_R4_Get_Model_ID(char* id){
  if(!SARA_R4_Send("at+cgmm\r\n"))
    return SARA_ERROR;
  return SARA_R4_Receive(id, true, 1000);
}

SARA_R4_Status_t SARA_R4_Get_Firmware_ID(char* id){
  if(!SARA_R4_Send("at+cgmr\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);
}

SARA_R4_Status_t SARA_R4_Get_IMEI(char* id){
  if(!SARA_R4_Send("at+gsn\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);
}

SARA_R4_Status_t SARA_R4_Get_IMSI(char* id){
  if(!SARA_R4_Send("at+cimi\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);
}

SARA_R4_Status_t SARA_R4_Get_ICCID(char* id){
    if(!SARA_R4_Send("at+ccid\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);
}

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


/* Low Level Functions */
bool SARA_R4_Send(char* msg){
  uint16_t len = strlen(msg);

  bool ret = Queue_IsEmpty(&cell_tx_queue);
  if(ret){
    for (uint16_t i = 1; i < len; i++){
      if(Queue_Append(&cell_tx_queue, msg + i) == QUEUE_FULL){
        return false; //TODO you should undo the mess you just made in the buffer
      }
    }
    huart->Instance->TDR = msg[0];
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn on TX interrupts
  }
  return ret;
}

SARA_R4_Status_t SARA_R4_Receive(char* msg, bool checkOK, uint16_t timeout){  //check at_ok, timeout in ms
  do{
    if(!Queue_IsEmpty(&message_queue))
    {
      SARA_R4_Resp_t resp;
      if(Queue_Get(&message_queue, &resp) == QUEUE_OK){

        if(resp.msgType == AT_COMMAND_RESPONSE || resp.msgType == AT_COMMAND_UNKNOWN){ //Temporary
          memcpy(msg, resp.message, strlen(resp.message));
          break;

        }else if(resp.msgType == AT_COMMAND_ERROR){
          return SARA_ERROR;
        }
      }
    }
    HAL_Delay(1);
  }while(timeout-- > 0); //Timeout

  if(checkOK){ //Wait for next message

  }
  return SARA_OK;
}

void SARA_R4_IRQ_Handler(UART_HandleTypeDef *huart)
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
    uint8_t c = huart->Instance->RDR; // Read out data
    if(tempMsg.len == 0){
      if(c == '\r' || c == '\n' || c == '\0') // Skip any EOL/junk characters
        return;
      else if(c == 'A' || c == 'a')
        tempMsg.msgType = AT_COMMAND_ECHO;
      else if(c == '+')
        tempMsg.msgType = AT_COMMAND_RESPONSE;
      else if(c == 'O' || c == 'o')
        tempMsg.msgType = AT_COMMAND_OK;
      else if(c == 'E' || c == 'e')
        tempMsg.msgType = AT_COMMAND_ERROR;
      else
        tempMsg.msgType = AT_COMMAND_UNKNOWN;

    // Send message when EOL character received
    }else if(c == '\r' || c == '\n'){
      tempMsg.message[tempMsg.len++] = '\0';
      Queue_Append(&message_queue, &tempMsg);   //Switch queues if async message comes in TODO
      tempMsg.len = 0;
      return;
    }

    // Store message if didn't skip or send in queue
    tempMsg.message[tempMsg.len++] = (char)c;
  }

  // TX UART ISR (TXEIE must be enabled)
  if (isrflags & USART_ISR_TXE && huart->Instance->CR1 & USART_CR1_TXEIE)
  {
    uint8_t tx;
    if (Queue_Get(&cell_tx_queue, &tx) == QUEUE_OK) // Push new data
      huart->Instance->TDR = tx;
    else{
      CLEAR_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn off TX interrupts
    }
  }
}

static void Process_AT_Command(void){ //Should pass pointer to queue message to process

}




