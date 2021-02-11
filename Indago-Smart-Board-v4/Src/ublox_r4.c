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
#include <stdio.h>

#include "ublox_r4.h"
#include "queue.h"
#include "main.h"
#include "print.h"
#include "aws.h"



// Globals
static UART_HandleTypeDef *huart;
Queue_t cell_tx_queue;//, cell_rx_queue;
Queue_t message_queue;

SARA_R4_Resp_t tempMsg;

char testing[30];

// Prototype functions
uint32_t Convert_From_ASCII(char *msg, uint8_t len);


/** MAIN UBLOX FUNCTIONS **/

//Init
bool SARA_R4_Init(UART_HandleTypeDef *uart){
  huart = uart;

  // Initialize Queues
  if(Queue_Init(&cell_tx_queue, sizeof(uint8_t), TX_BUFFER_SIZE) != QUEUE_OK)
    return false;
//  if(Queue_Init(&cell_rx_queue, sizeof(uint8_t), RX_BUFFER_SIZE) != QUEUE_OK)
//    return false;
  if(Queue_Init(&message_queue, sizeof(SARA_R4_Resp_t), MSG_BUFFER_SIZE) != QUEUE_OK)
    return false;

  // Initialize UART HW //TODO MOVE THIS STATEMENT
  SET_BIT(huart->Instance->RQR, UART_RXDATA_FLUSH_REQUEST);                     // Clear RX ISR flags
  SET_BIT(huart->Instance->ICR, 0xFFFF);                                        // Clear all flags
  SET_BIT(huart->Instance->CR3, USART_CR3_EIE);                                 // Turn on RNE IRQ (Main UART IRQ)
  SET_BIT(huart->Instance->CR1, USART_CR1_RXNEIE);

#if PASS_THROUGH_UART
  while(1)
    ;
#endif

  // Test functionality w/ standard device information
//  SARA_R4_Get_Manufacturer_ID(testing);
//  HAL_Delay(50);
//  SARA_R4_Get_Model_ID(testing);
//  HAL_Delay(50);
//  SARA_R4_Get_Firmware_ID(testing);
//  HAL_Delay(50);
//  SARA_R4_Get_IMEI(testing);
//  HAL_Delay(50);
//  SARA_R4_Get_IMSI(testing);
//  HAL_Delay(50);
  SARA_R4_Get_ICCID(testing);
  HAL_Delay(100);

  //Setup the network etc
  // Start storing the files
#if AWS_LOAD_CA
  SARA_R4_Download_Block("rootca", (char*)AWS_ROOT_CA, strlen(AWS_ROOT_CA));
  HAL_Delay(50);
  SARA_R4_Download_Block("privateca", (char*)AWS_PRIVATE_CA, strlen(AWS_PRIVATE_CA));
  HAL_Delay(50);
  SARA_R4_Download_Block("privatekey", (char*)AWS_PRIVATE_KEY, strlen(AWS_PRIVATE_KEY));
#endif //AWS_LOAD_CA

  HAL_Delay(50);
  SARA_R4_List_File("rootca");
  HAL_Delay(50);
  SARA_R4_List_File("privateca");
  HAL_Delay(50);
  SARA_R4_List_File("privatekey");
  HAL_Delay(50);

  SARA_R4_Hex_Mode(1);
  HAL_Delay(50);

  uint8_t profile_id = 0;
  SARA_R4_CPRF_Set_TLS(profile_id, 0, 0, "rootca", "privateca", "privatekey");
  HAL_Delay(50);

  HAL_Delay(50);
  SARA_R4_Operator_t oper;
  oper.status = 0;
  while(oper.status == 0 || oper.status == 2){
    SARA_R4_Get_Operator(&oper);
    HAL_Delay(100);
  }
  HAL_Delay(50);
  SARA_R4_Get_RAT(0);
  HAL_Delay(50);
  SARA_R4_Get_Network_Registration();
  HAL_Delay(50);
  SARA_R4_Get_Network_Selection();
  HAL_Delay(50);

  uint8_t socket;
  SARA_R4_Create_Socket(6, &socket);

  HAL_Delay(50);
  SARA_R4_SSL_Mode_Config(socket, 1, profile_id);

  HAL_Delay(100);
  SARA_R4_Connect_Socket(socket, (char*)AWS_ADDR, AWS_PORT);

  HAL_Delay(150);
  char send[150] = "AT+USOWR=0,32,\"301E0009746573742F64617465323032302D31312D32335432303A34363A3235\"\r\n";
  SARA_R4_Send(send);

  HAL_Delay(150);
  SARA_R4_Send(send);

  //HAL_Delay(5000);
  //SARA_R4_Send(send);

//  static uint8_t rxlev, ber, rscp, enc0, rsrq, rsrp;
//  SARA_R4_Status_t ret = SARA_R4_Get_Extended_Signal_Quality(&rxlev, &ber, &rscp, &enc0, &rsrq, &rsrp);
//  if(ret != SARA_OK)
//    Error_Handler();
//  printf("%d, %d, %d, %d, %d, %d", rxlev, ber, rscp, enc0, rsrq, rsrp);



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

  return SARA_OK;
}

// Ublox Phone Settings
SARA_R4_Status_t SARA_R4_Get_Manufacturer_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmi\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("Manufacturer ID: ");
  printf(msg.message);
  printf("\n");

  // Do something with the id
  // Store and manipulate

  return ret;
}
SARA_R4_Status_t SARA_R4_Get_Model_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmm\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("Model ID: ");
  printf(msg.message);
  printf("\n");

  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_Firmware_ID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgmr\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("Firmware ID: ");
  printf(msg.message);
  printf("\n");

  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_IMEI(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cgsn\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("IMEI: ");
  printf(msg.message);
  printf("\n");

  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_IMSI(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cimi\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("IMSI: ");
  printf(msg.message);
  printf("\n");

  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_ICCID(char *id){
  SARA_R4_Status_t ret = SARA_R4_Send("at+ccid\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf("ICCID: ");
  printf(msg.message);
  printf("\n");

  return SARA_OK;
}

/* UBLOX PHONE CONFIGURATION */
SARA_R4_Status_t SARA_R4_Shutdown(void){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cpwroff\r\n");
    if(ret != SARA_OK)
      return ret;

    return SARA_R4_Receive(AT_COMMAND_OK, NULL, 1000);
}

SARA_R4_Status_t SARA_R4_Get_Functionality_Level(uint8_t *power){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cfun?\r\n");
  if(ret != SARA_OK)
    return ret;

  char msg[10];
//  ret = SARA_R4_Receive(AT_COMMAND_OK, msg, 1000);
//  if(ret != SARA_OK)
//    return ret;

  *power = msg[6]; // This is the ascii representation
  return ret;
}

SARA_R4_Status_t SARA_R4_Set_Functionality_Level(uint8_t fun, bool reset){
//    SARA_R4_Status_t ret;// TODO = SARA_R4_Send(sprintf("at+cfun=%d,%d\r\n", fun, reset));
//    if(ret != SARA_OK)
//      return ret;
//
//    return SARA_R4_Receive(AT_COMMAND_OK, NULL, 1000);
  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_Indications(SARA_R4_Indication_t *status){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cind?\r\n");
    if(ret != SARA_OK)
      return ret;

    char msg[50];
//    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
//    if(ret != SARA_OK)
//      return ret;

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

SARA_R4_Status_t SARA_R4_Set_Event_Reporting(void){return SARA_OK;} //CMER?
SARA_R4_Status_t SARA_R4_Set_Timezone_Update(void){return SARA_OK;}

/* UBLOX CELLULAR NETWORK REGISTRATION */
SARA_R4_Status_t SARA_R4_Get_Subscriber_Number(void){return SARA_OK;}

SARA_R4_Status_t SARA_R4_Get_Signal_Quality(uint8_t *dbm){
    SARA_R4_Status_t ret = SARA_R4_Send("at+csq\r\n");
    if(ret != SARA_OK)
      return ret;

    char msg[10];
//    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, msg, 1000);
//    if(ret != SARA_OK)
//      return ret;


    // Need to map values
    // Need to use string split to parse the numbers out of the commas
    *dbm = msg[5];
    return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_Extended_Signal_Quality(uint8_t *rxlev, uint8_t *ber, uint8_t *rscp, uint8_t *enc0, uint8_t *rsrq, uint8_t *rsrp){
    SARA_R4_Status_t ret = SARA_R4_Send("at+cesq\r\n");
    if(ret != SARA_OK)
      return ret;

    SARA_R4_Resp_t msg;
    ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
    if(ret != SARA_OK)
      return ret;


    // Need to use string split to parse the numbers out of the commas
    char sub_msg[4];
    char* idx1, *idx2;
    idx1 = strchr(msg.message, ':'); // Find the start of the response
    idx2 = strchr(idx1, ',');

    memcpy(sub_msg, idx1+2, idx2-idx1-2);       // First set has an extra space
    sub_msg[idx2-idx1-2] = '\0';
    *rxlev = atoi(sub_msg);

    idx1 = idx2 + 1;
    idx2 = strchr(idx1, ',');
    memcpy(sub_msg, idx1, idx2 - idx1); // idx1 is indexed to first number
    sub_msg[idx2-idx1] = '\0';
    *ber = atoi(sub_msg);

    idx1 = idx2 + 1;
    idx2 = strchr(idx1, ',');
    memcpy(sub_msg, idx1, idx2 - idx1); // idx1 is indexed to first number
    sub_msg[idx2-idx1] = '\0';
    *rscp = atoi(sub_msg);

    idx1 = idx2 + 1;
    idx2 = strchr(idx1, ',');
    memcpy(sub_msg, idx1, idx2 - idx1); // idx1 is indexed to first number
    sub_msg[idx2-idx1] = '\0';
    *enc0 = atoi(sub_msg);

    idx1 = idx2 + 1;
    idx2 = strchr(idx1, ',');
    memcpy(sub_msg, idx1, idx2 - idx1); // idx1 is indexed to first number
    sub_msg[idx2-idx1] = '\0';
    *rsrq = atoi(sub_msg);

    idx1 = idx2 + 1;
    idx2 = msg.message + msg.len;
    memcpy(sub_msg, idx1, idx2 - idx1); // idx1 is indexed to first number
    sub_msg[idx2-idx1] = '\0';
    *rsrp = atoi(sub_msg);

    return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Get_RAT(uint8_t *rat){
  SARA_R4_Status_t ret = SARA_R4_Send("at+urat?\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf(msg.message);
  printf("\n");

  return SARA_OK;

    //uint8_t idx = strchr(msg, ':') + 1;
    //*rat = msg[idx] - 0x30;
}

SARA_R4_Status_t SARA_R4_Get_Operator(SARA_R4_Operator_t *oper){
  SARA_R4_Status_t ret = SARA_R4_Send("at+cops?\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  oper->status = msg.message[9];

  printf(msg.message);
  printf("\n");

  //GET OPERATOR HERE

  return SARA_OK;
}
SARA_R4_Status_t SARA_R4_Set_Operator(SARA_R4_Operator_t *oper){return SARA_OK;}

SARA_R4_Status_t SARA_R4_Get_Network_Registration(void){
  SARA_R4_Status_t ret = SARA_R4_Send("at+creg?\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf(msg.message);
  printf("\n");

  return SARA_OK;
}
SARA_R4_Status_t SARA_R4_Set_Network_Registration(void){return SARA_OK;}

SARA_R4_Status_t SARA_R4_Get_Network_Selection(void){
  SARA_R4_Status_t ret = SARA_R4_Send("at+ucged?\r\n");
  if(ret != SARA_OK)
    return ret;

  SARA_R4_Resp_t msg;
  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  printf(msg.message);
  printf("\n");
  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_Set_Network_Selection(void){return SARA_OK;}

SARA_R4_Status_t SARA_R4_Get_Band_Selection(void){return SARA_OK;}
SARA_R4_Status_t SARA_R4_Set_Band_Selection(void){return SARA_OK;}


/* SIM MANAGEMENT */


/* INTERNET AND SECURITY CONFIGURATION */
// SSL/TLS Certification / Key Management
//void SARA_R4_ TODO:10/20/20
// Security Layer Profile Management
SARA_R4_Status_t SARA_R4_CPRF_Set_TLS(uint8_t profile, uint8_t ssl_level, uint8_t cipher_suite, char *root_cert, char *client_cert, char *client_key){
  SARA_R4_Status_t ret;
  SARA_R4_Resp_t msg;

  // Set the root cert
  char send[50];
  sprintf(send, "at+usecmng=1,0,\"%s\",\"%s\"\r\n", root_cert, root_cert);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;
  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
  if(ret != SARA_OK)
    return ret;
  HAL_Delay(100);

  // Set the client cert
  sprintf(send, "at+usecmng=1,1,\"%s\",\"%s\"\r\n", client_cert, client_cert);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  HAL_Delay(100);

  // Set the client key
  sprintf(send, "at+usecmng=1,2,\"%s\",\"%s\"\r\n", client_key, client_key);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  HAL_Delay(100);

  sprintf(send, "at+usecmng=3\r\n");
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
  if(ret != SARA_OK)
    return ret;

  HAL_Delay(100);

  // Set the certificate validation level
  sprintf(send, "at+usecprf=%d,0,0\r\n", profile);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  HAL_Delay(100);

  // Set SSL/TLS Version
  sprintf(send, "at+usecprf=%d,1,%d\r\n", profile, ssl_level);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  HAL_Delay(100);

  // Set Cipher Suite
  sprintf(send, "at+usecprf=%d,2,%d\r\n", profile, cipher_suite);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  HAL_Delay(100);

  // Set RootCA
  sprintf(send, "at+usecprf=%d,3,\"%s\"\r\n", profile, root_cert);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  HAL_Delay(100);

  // Set ClientCert
  sprintf(send, "at+usecprf=%d,5,\"%s\"\r\n", profile, client_cert);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  HAL_Delay(100);

  // Set ClientKey
  sprintf(send, "at+usecprf=%d,6,\"%s\"\r\n", profile, client_key);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;

  return SARA_OK;
}

// HEX Mode Configuration
SARA_R4_Status_t SARA_R4_Hex_Mode(uint8_t enable){
  SARA_R4_Status_t ret;
  SARA_R4_Resp_t msg;

  char send[25];
  sprintf(send, "at+udconf=1,%d\r\n", enable);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  return SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
}

// Socket Connection
SARA_R4_Status_t SARA_R4_Clear_Socket(uint8_t socket){return SARA_OK;}
SARA_R4_Status_t SARA_R4_Create_Socket(uint8_t protocol, uint8_t* socket){
  SARA_R4_Status_t ret;
  SARA_R4_Resp_t msg;

  char send[20] = "at+usocr=";
  sprintf(send, "at+usocr=%d\r\n", protocol);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  ret = SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 2000);
  if(ret != SARA_OK)
    return ret;

  // Store the socket number to be used
  *socket = msg.message[8] - 48; // Double check this is the right index
  //printf("%d",*socket);
  return SARA_OK;
}

SARA_R4_Status_t SARA_R4_SSL_Mode_Config(uint8_t socket, uint8_t ssl_enable, uint8_t profile_id){
  SARA_R4_Status_t ret;
  SARA_R4_Resp_t msg;

  char send[24];
  sprintf(send, "at+usosec=%d,%d,%d\r\n", socket, ssl_enable, profile_id);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  // Optional check the message for errors
  return SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
}

SARA_R4_Status_t SARA_R4_Setup_Socket(void){return SARA_OK; } // TODO:

SARA_R4_Status_t SARA_R4_Connect_Socket(uint8_t socket, char* addr, uint16_t port){ //TODO: Optional async param
  SARA_R4_Status_t ret;
  SARA_R4_Resp_t msg;

  char send[60];
  sprintf(send, "at+usoco=%d,\"%s\",%d\r\n", socket, addr, port);
  ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  // Check for connection
  return SARA_R4_Receive(AT_COMMAND_RESPONSE, &msg, 5000); //TODO: Debug the responses here*/
}

SARA_R4_Status_t SARA_R4_Write_Socket(uint8_t socket, uint8_t *data, uint16_t len){
  return SARA_OK;
}

/* FILE SYSTEM MANAGEMENT */
// Clears the files in the file system
SARA_R4_Status_t SARA_R4_Clear_File(char* f_name){
  char send[50] = "at+udelfile=\"";
  strncat(send, f_name, strlen(f_name));
  strncat(send, "\"\r\n", 3);

  SARA_R4_Status_t ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  // Optional check the message for errors
  SARA_R4_Resp_t msg;
  return SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
}

// List the file
//TODO: Add the op_code
SARA_R4_Status_t SARA_R4_List_File(char* f_name){
  char send[50] = "at+ulstfile=2,\"";
  strncat(send, f_name, strlen(f_name));
  strncat(send, "\"\r\n", 3);

  SARA_R4_Status_t ret = SARA_R4_Send(send);
  if(ret != SARA_OK)
    return ret;

  // Optional check the message for errors
  SARA_R4_Resp_t msg;
  return SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
}

// Downloads the whole file
// Maximum file name length 248B
// Not recommended if HW flow control not enabled, and will need to manage TX buffer size depending on file size
SARA_R4_Status_t SARA_R4_Download_File(char* f_name, char* data, uint16_t data_len){
  return SARA_OK;

}

// Downloads a file into the file system
// Automatically splits the data up into blocks
SARA_R4_Status_t SARA_R4_Download_Block(char* f_name, char* data, uint16_t data_len){
  // Clear the file first
  SARA_R4_Clear_File(f_name);
  HAL_Delay(50);

  char snd[50] = "at+udwnblock=\""; //TODO CHECK THE MAX LENGTH OF THE FILENAME
  strncat(snd, f_name, strlen(f_name));
  uint8_t prefix_len = strlen(snd);

  // Loop through this n times in 100 byte chunks until all the data is sent
  uint8_t i;
  for(i = 0; i < (data_len / 100)+1; i++)
  {
    uint8_t block_size =  i*100+100 < data_len ? 100 : data_len - i*100;

    char str[20] = "\",";
    sprintf(str, "\",%d,%d,%d\r\n", i*100, block_size, data_len); // Format: <filename>,<offset>,<size>,<filesize>
    strcpy(snd+prefix_len, str);

    // Send the first command
    SARA_R4_Status_t ret = SARA_R4_Send(snd);
    if(ret != SARA_OK)
      return ret;

    // Check device is ready to receive
    SARA_R4_Resp_t msg;
    ret = SARA_R4_Receive(AT_COMMAND_INPUT, &msg, 1000); //Expecting a '>'
    if(ret != SARA_OK)
      return ret;

    // Send the block
    SARA_R4_SendN(data+i*100, block_size);
    ret = SARA_R4_SendN("\r\n", 2);
    if(ret != SARA_OK)
      return ret;

    // Verify receipt of data
    msg.message[0] = '\0';
    ret = SARA_R4_Receive(AT_COMMAND_OK, &msg, 1000);
    if(ret != SARA_OK)
      return ret;
  }

  return SARA_OK;

}

/* Low Level Functions */
SARA_R4_Status_t SARA_R4_Send(char* msg){
  uint16_t len = strlen(msg);
  bool ret = Queue_IsEmpty(&cell_tx_queue);
  if(ret){
    for (uint16_t i = 1; i < len; i++){
      if(Queue_Append(&cell_tx_queue, msg + i) == QUEUE_FULL){ //TODO This is inefficient
        return SARA_ERROR; //TODO you should undo the mess you just made in the buffer
      }
    }
    huart->Instance->TDR = msg[0];
    SET_BIT(huart->Instance->CR1, USART_CR1_TXEIE); // Turn on TX interrupts
  }
  return ret ? SARA_OK : SARA_ERROR;
}

SARA_R4_Status_t SARA_R4_SendN(char* msg, uint16_t len){
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
  else{
    for(uint16_t i = 0; i < len; i++){
      if(Queue_Append(&cell_tx_queue, msg + i) == QUEUE_FULL){
        return SARA_ERROR; //TODO you should undo the mess you just made in the buffer
      }
    }
  }
  return SARA_OK;
}


SARA_R4_Status_t SARA_R4_Receive(SARA_R4_Msg_t msgType, SARA_R4_Resp_t* msg, uint16_t timeout){  // Message type to look for, response
  do{
    if(!Queue_IsEmpty(&message_queue))
    {
      if(Queue_Get(&message_queue, msg) == QUEUE_OK){
        if(msg->msgType == msgType && msg->len > 1){    // Temporary
          return SARA_OK;
        }else if(msg->msgType == AT_COMMAND_ERROR){
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
      else if(c == '>'){                // Special exception: One character message, waiting on input
        tempMsg.msgType = AT_COMMAND_INPUT;
        tempMsg.message[tempMsg.len++] = c;
        tempMsg.message[tempMsg.len++] = '\0';
        Queue_Append(&message_queue, &tempMsg);
        tempMsg.len = 0;
        return;
      }
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

uint32_t Convert_From_ASCII(char *msg, uint8_t len){
  uint8_t i;
  uint32_t total = 0;
  uint32_t multiplier = 1;
  for(i = len - 1; i >= 0; i--){
    total += multiplier * (msg[i] - 0x30);
    multiplier *= 10;
  }
  return total;
}