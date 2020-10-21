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
#include "aws.h"

// Globals
static UART_HandleTypeDef *huart;
Queue_t cell_tx_queue, cell_rx_queue;
Queue_t message_queue;

SARA_R4_Resp_t tempMsg;

char testing[30];

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

  // Test functionality w/ standard device information 
  SARA_R4_Get_Manaufacturer_ID(testing);
  /*SARA_R4_Get_Model_ID(testing);
  SARA_R4_Get_Firmware_ID(testing);
  SARA_R4_Get_IMEI(testing);
  SARA_R4_Get_IMSI(testing);
  SARA_R4_Get_ICCID(testing);*/


  //Setup the network etc
  // Start storing the files
  SARA_R4_Status_t status = SARA_R4_Download_Block("rootca", AWS_ROOT_CA, strlen(AWS_ROOT_CA));
  //Now do this for the private key



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

// Hex Mode Configuration
SARA_R4_Status_t SARA_R4_Hex_Mode_Config(uint8_t mode){
  //sprintf
  SARA_R4_Status_t ret = SARA_R4_Send("at+udconf=1");
}

// Ublox Phone Settings
SARA_R4_Status_t SARA_R4_Get_Manufacturer_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmi\r\n");
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, id, 1000);
  if(ret != SARA_OK)
    return ret;

  // Do something with the id
  // Store and manipulate

  return ret;
}
SARA_R4_Status_t SARA_R4_Get_Model_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmm\r\n");
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, id, 1000);
  if(ret != SARA_OK)
    return ret;
}

SARA_R4_Status_t SARA_R4_Get_Firmware_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmm\r\n");
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, id, 1000);
  if(ret != SARA_OK)
    return ret;
}

SARA_R4_Status_t SARA_R4_Get_IMEI(char *id){
  /*if(!SARA_R4_Send("at+gsn\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);*/
}

SARA_R4_Status_t SARA_R4_Get_IMSI(char *id){
  /*if(!SARA_R4_Send("at+cimi\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);*/
}

SARA_R4_Status_t SARA_R4_Get_ICCID(char *id){
    /*if(!SARA_R4_Send("at+ccid\r\n"))
      return SARA_ERROR;
    return SARA_R4_Receive(id, true, 1000);*/
}

/* UBLOX PHONE CONFIGURATION */
void SARA_R4_Shutdown(void){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cpwroff\r\n");
    if(ret != SARA_OK)
      return ret;

    return SARA_R4_Receive(AT_COMMAND_OK, NULL, 1000);
}

void SARA_R4_Get_Functionality_Level(uint8_t *power){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cfun?\r\n");
  if(ret != SARA_OK)
    return ret;

  uint8_t msg[10];
  ret = SARA_R4_Receive(AT_COMMAND_OK, msg, 1000);
  if(ret != SARA_OK)
    return ret;

  *power = msg[6]; // This is the ascii representation
  return ret;
}

void SARA_R4_Set_Functionality_Level(uint8_t fun, bool reset){
    SARA_R4_Status_t ret = SARA_R4_Send(sprintf("at+cfun=%d,%d\r\n",fun, reset));
    if(ret != SARA_OK)
      return ret;

    return SARA_R4_Receive(AT_COMMAND_OK, NULL, 1000);
}

void SARA_R4_Get_Indications(SARA_R4_Indication_t *status){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cind?\r\n");
    if(ret != SARA_OK)
      return ret;

    uint8_t msg[50];
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
    if(ret != SARA_OK)
      return ret;

    status->battchg = msg[6]; //Should include some checks. This is placing ascii representations. Should convert.
    status->signal = msg[8];
    status->service = msg[10];
    status->sounder = msg[12];
    status->message = msg[14];
    status->call = msg[16];
    status->roam = msg[18];
    status->smsfull = msg[20];
    status->gprs = msg[22];
    status->callsetup = msg[24];
    status->callheld = msg[26];
    status->simind = msg[28];

    return ret;
}

void SARA_R4_Set_Event_Reporting(void){} //CMER?
void SARA_R4_Set_Timezone_Update(void){}

/* UBLOX CELLULAR NETWORK REGISTRATION */
void SARA_R4_Get_Subscriber_Number(void){}

void SARA_R4_Get_Signal_Quality(uint8_t *dbm){
    SARA_R4_Status_t ret = SARA_R4_Send("at+csq\r\n");
    if(ret != SARA_OK)
      return ret;

    uint8_t msg[10];
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
    if(ret != SARA_OK)
      return ret;


    // Need to map values
    // Need to use string split to parse the numbers out of the commas
    *dbm = msg[5];
}

void SARA_R4_Get_Extended_Signal_Quality(uint8_t *rxlev, uint8_t *ber, uint8_t *rscp, uint8_t *enc0, uint8_t *rsrq, uint8_t *rsrp){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cesq\r\n");
    if(ret != SARA_OK)
      return ret;

    uint8_t msg[25];
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
    if(ret != SARA_OK)
      return ret;

    //TODO: MAP VALUES
    //TODO: REVIEW THE ERROR CHECKING
    //TODO: TEST THIS PARSING
    //TOOD: MODIFY THE OTHER COMMANDS TO USE THE : SEARCH TO INDICATE START OF THE DATA FROM THE MESSAGE
    //TODO: CHANGE THE 0x30 to a #DEFINE

    // Need to use string split to parse the numbers out of the commas
    uint8_t idx1 = strchr(msg,':') + 1; // Find the start of the response

    if(strlen(msg) - idx1 < 20) // Check the length before parsing
      return SARA_ERROR;

    // Start parsing the message
    uint8_t idx2 = strchr(msg, ',');
    *rxlev = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);

    idx1 = idx2 + 1;
    idx2 = strchr(msg + idx1, ',');
    *ber = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);

    idx1 = idx2 + 1;
    idx2 = strchr(msg + idx1, ',');
    *rscp = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);

    idx1 = idx2 + 1;
    idx2 = strchr(msg + idx1, ',');
    *enc0 = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);

    idx1 = idx2 + 1;
    idx2 = strchr(msg + idx1, ',');
    *rsrq = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);

    idx1 = idx2 + 1;
    idx2 = strchr(msg + idx1, ',');
    *rsrp = (uint8_t)Convert_From_ASCII(msg+idx1, idx2-idx1);
}

void SARA_R4_Get_RAT(uint8_t *rat){
    SARA_R4_Status_t ret = SARA_R4_Send("at+urat?\r\n");
    if(ret != SARA_OK)
      return ret;

    uint8_t msg[10];
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
    if(ret != SARA_OK)
      return ret;

    uint8_t idx = strchr(msg, ':') + 1;
    *rat = msg[idx] - 0x30;
}

void SARA_R4_Get_Operator(SARA_R4_Operator_t *oper){
    SARA_R4_Status_t ret = SARA_R4_Send("at+urat?\r\n");
      if(ret != SARA_OK)
        return ret;

    uint8_t msg[40];
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
    if(ret != SARA_OK)
      return ret;


}
void SARA_R4_Set_Operator(SARA_R4_Operator_t *oper){}

void SARA_R4_Get_Network_Registration(void){}
void SARA_R4_Set_Network_Registration(void){}

void SARA_R4_Get_Network_Selection(void){}
void SARA_R4_Set_Network_Selection(void){}

void SARA_R4_Get_Band_Selection(void){}
void SARA_R4_Set_Band_Selection(void){}


/* SIM MANAGEMENT */


/* INTERNET AND SECURITY CONFIGURATION */
// SSL/TLS Certification / Key Management 
void SARA_R4_ TODO:10/20/20
// Security Layer Profile Management
void SARA_R4_CPRF_Set_TLS(uint8_t profile, uint8_t ssl_level, uint8_t cipher_suite, string root_cert, string client_cert, string client_key){
  SARA_R4_Status_t ret;

  // Set the cert validation
  ret = SARA_R4_Send("at+urat?\r\n");
  if(ret != SARA_OK)
    return ret;

  

  // Set the tls version
  // Set the cipher suite
  // Set the root cert
  // Set the client cert
  // Set the client key
}

// Socket Connection
uint8_t SARA_R4_Clear_Socket(uint8_t socket){}
uint8_t SARA_R4_Create_Socket(uint8_t protocol, uint8_t* socket){
  SARA_R4_Status_t ret;

  string send = "at+usocr=";
  sprintf(send, "%d\r\n", protocol);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  char msg[10];
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 2000);
  if(ret != SARA_OK)
    return ret;

  // Store the socket number to be used
  *socket = msg[6]; // Double check this is the right index

  return SARA_OK;
}

void SARA_R4_SSL_Mode_Config(uint8_t socket, uint8_t ssl_enable, uint8_t profile_id){
  SARA_R4_Status_t ret;

  string send = "at+usosec=";
  sprintf(send, "%d,%d,%d\r\n", socket, ssl_enable, profile_id);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  // Optional check the message for errors
  char msg[10];
  return SARA_R4_Receive(AT_COMMAND_OK, msg, 1000); 
}

void SARA_R4_Setup_Socket(void){ } // TODO: 

void SARA_R4_Connect_Socket(uint8_t socket, char* addr, uint16_t port){ //TODO: Optional async param
  SARA_R4_Status_t ret;
  string send = "at+usoco=";
  sprintf(send, "%d,%s,%d\r\n");
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;
  
  // Check for connection
  char msg[50];
  return SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 10000); //TODO: Debug the responses here
}

void SARA_R4_Write_Socket(uint8_t socket, uint8_t *data, uint16_t len){

}

/* FILE SYSTEM MANAGEMENT */
// Clears the files in the file system
SARA_R4_Status_t SARA_R4_Clear_Files(void){

}

// Downloads the whole file
// Maximum file name length 248B
// Not recommended if HW flow control not enabled, and will need to manage TX buffer size depending on file size
SARA_R4_Status_t SARA_R4_Download_File(char* f_name, char* data, uint16_t data_len){


}

// Downloads a file into the file system
// Automatically splits the data up into blocks
SARA_R4_Status_t SARA_R4_Download_Block(char* f_name, char* data, uint16_t data_len){
  string cmd_prefix = "at+udwnblock=\"";
  strncat(cmd_prefix, f_name, strlen(f_name));

  // Loop through this n times in 100 byte chunks until all the data is received
  uint8_t i;
  for(i = 0; i < (data_len / 100)+1; i++)
  {
    uint8_t block_size =  i*100 < data_len ? 100 : data_len - i*100;
    string send;
    string cmd2 = "\",0,";
    sprintf(cmd2, "%d\r\n", block_size); // Use the block size or what is left over

    strcpy(send, cmd_prefix);
    strcat(send, cmd2);

    // Send the first command
    SARA_R4_Status_t ret = SARA_R4_Send(send);
    if(ret != SARA_OK)
      return ret;

    // Check device is ready to receive
    uint8_t msg[10];
    ret = SARA_R4_Receive(AT_COMMAND_UNKNOWN, msg, 1000); //Expecting a '>'
    if(ret != SARA_OK)
      return ret;

    // Send the block
    SARA_R4_Status_t ret = SARA_R4_SendN(data+i*100, block_size);
    if(ret != SARA_OK)
      return ret;


    // Verify receipt of data
    uint8_t msg[10];
    ret = SARA_R4_Receive(AT_COMMAND_OK, msg, 1000);
    if(ret != SARA_OK)
      return ret;
  }

}

/* Low Level Functions */
SARA_R4_Status_t SARA_R4_Send(char* msg){
  uint16_t len = strlen(msg);

  bool ret = Queue_IsEmpty(&cell_tx_queue);
  if(ret){
    for (uint16_t i = 1; i < len; i++){
      if(Queue_Append(&cell_tx_queue, msg + i) == QUEUE_FULL){
        return SARA_ERROR; //TODO you should undo the mess you just made in the buffer
      }
    }
    huart->Instance->TDR = msg[0];
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn on TX interrupts
  }
  return ret ? SARA_OK : SARA_ERROR;
}

SARA_R4_Status_t SARA_R4_Send(char* msg, uint16_t len){
  bool ret = Queue_IsEmpty(&cell_tx_queue);
  if(ret){
    for (uint16_t i = 1; i < len; i++){
      if(Queue_Append(&cell_tx_queue, msg + i) == QUEUE_FULL){
        return SARA_ERROR; //TODO you should undo the mess you just made in the buffer
      }
    }
    huart->Instance->TDR = msg[0];
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn on TX interrupts
  }
  return ret ? SARA_OK : SARA_ERROR;
}

SARA_R4_Status_t SARA_R4_Receive(SARA_R4_Msg_t msgType, char* msg, uint16_t timeout){  // Message type to look for, response
  do{
    if(!Queue_IsEmpty(&message_queue))
    {
      SARA_R4_Resp_t resp;
      if(Queue_Get(&message_queue, &resp) == QUEUE_OK){

        if(resp.msgType == msgType){ //Temporary
          if(msg != NULL)
            memcpy(msg, resp.message, strlen(resp.message));
          break;

        }else if(resp.msgType == AT_COMMAND_ERROR){
          return SARA_ERROR;
        }
      }
    }
    HAL_Delay(1);
  }while(timeout-- > 0); //Timeout

  return SARA_TIMEOUT;
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

uint32_t Convert_From_ASCII(uint8_t *msg, uint8_t len){
  uint8_t i;
  uint32_t total = 0;
  uint32_t multiplier = 1;
  for(i = len - 1; i > 0; i--){
    total += multiplier * (msg[i] - 0x30);
    multiplier *= 10;
  }
  return total;
}




