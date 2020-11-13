
#include "stm32l4xx_hal.h"
#include "print.h"

extern UART_HandleTypeDef huart1; //MAX-8Q

/* FILE is typedef’d in stdio.h. */
FILE __stdout;

int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFFFF);
  return ch;
}

int ferror(FILE *f)
{
  /* Your implementation of ferror(). */
  return 0;
}