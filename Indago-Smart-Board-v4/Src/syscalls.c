#include <stdint.h>
#include <stdio.h>

#include "stm32l4xx_hal.h"


extern UART_HandleTypeDef huart1;

int fputc(int ch, FILE *f){
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
  return ch;
}