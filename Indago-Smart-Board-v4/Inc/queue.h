/********************************************************************************
  *
  * @file       : queue.h
  * @brief      : Header file for generic light-weight ring buffer / queue library to hold
                  objects in a FIFO fashion.
  * @author     : Charles H
  * @date       : Feb 2020
  *
*********************************************************************************/

#ifndef __QUEUE_H
#define __QUEUE_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "stm32l4xx_hal.h"

/* Queue Definitions */

// Queue object for the ring buffer
/**
  * @brief
  */
typedef struct Queue
{
  uint8_t *object;
  uint16_t head;
  uint16_t tail;
  bool isEmpty;
  uint16_t len;
  uint16_t obj_size;
} Queue_t;

typedef Queue_t *QueueHandle_t;

/**
  * @brief
  */
typedef enum QueueStatus
{
  QUEUE_OK,
  QUEUE_EMPTY,
  QUEUE_FULL,
  QUEUE_FAILED,
} QueueStatus_t;

/* Function Declarations */
QueueStatus_t Queue_Init(Queue_t *queue, uint16_t obj_size, uint16_t len);
QueueStatus_t Queue_Append(Queue_t *queue, void *data);
QueueStatus_t Queue_Get(Queue_t *queue, void *data);

bool Queue_IsFull(Queue_t *queue);
bool Queue_IsEmpty(Queue_t *queue);

#endif //__QUEUE_H