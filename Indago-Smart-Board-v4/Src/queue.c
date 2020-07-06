/********************************************************************************
  * @file       : queue.c
  * @brief      : This is a generic light-weight ring buffer / queue library to hold
                  objects in a FIFO fashion. See the comments below for example implementation.
  * @author     : Charles H
  * @date       : Feb 2020
  *
  *******************************************************************************/

#include <stdlib.h>

#include "queue.h"

/**
  * @brief Initialize a new queue dynamically and set up the ring buffer
  * @param
  * @retval
  * @note None
  */
QueueStatus_t Queue_Init(Queue_t *queue, uint16_t obj_size, uint16_t len)
{
  // Malloc appropriate memory for queue length * CCB_Packet_t and queue struct
  queue->object = malloc(len * obj_size);
  if(queue->object == NULL)
    return QUEUE_FAILED;

  // Set parameters
  queue->len = len;
  queue->obj_size = obj_size;
  queue->head = 0;
  queue->tail = 0;
  queue->isEmpty = true;

  return QUEUE_OK;
}

/**
  * @brief Add an item to a specified FIFO queue. Check the return result in the case that the queue is full
            or the operation fails
  * @param
  * @retval
  * @note None
  */
QueueStatus_t Queue_Append(Queue_t *queue, void *data)
{
  if (Queue_IsFull(queue)) // If it's full, don't overwrite old data
    return QUEUE_FULL;

  memcpy(queue->object + queue->obj_size * queue->head++, data, queue->obj_size);

  if (queue->head == queue->len) // Reset head if over size of array
    queue->head = 0;
  if (queue->head != queue->tail)
    queue->isEmpty = false;
  return QUEUE_OK;
}

/**
  * @brief Get the first item out of a specified FIFO queue. Check the return result prior to using
            in the case that the queue is empty or the operation fails
  * @param
  * @retval
  * @note None
  */
QueueStatus_t Queue_Get(Queue_t *queue, void *data)
{
  if (!Queue_IsEmpty(queue)) // Only if not empty yet
    memcpy(data, queue->object + queue->obj_size * queue->tail++, queue->obj_size);
  else
    return QUEUE_EMPTY;

  if (queue->tail == queue->len) // Reset tail if over size of array
    queue->tail = 0;
  if (queue->tail == queue->head)
    queue->isEmpty = true;
  return QUEUE_OK;
}

/**
  * @brief Clear all items out of a specified FIFO queue. This doesn't clear memory,
            only resets the head and the tail pointers
  * @param
  * @retval
  * @note None
  */
void Queue_Clear(void)
{
}

/**
  * @brief Checks if the specified queue is empty
  * @param queue Handle the queue instance
  * @retval bool Is empty?
  * @note None
  */
bool Queue_IsEmpty(Queue_t *queue)
{
  return queue->isEmpty;
}

/**
  * @brief Checks if the specified queue is at max capacity
  * @param quuee Handle to the queue instance
  * @retval bool Is full?
  * @note None
  */
bool Queue_IsFull(Queue_t *queue)
{
  return ((queue->head == queue->tail) && !queue->isEmpty);
}

/**
  * @brief Checks the current size of the specified queue
  * @param queue Handle to a queue instance
  * @retval uint16_t Current size of queue
  * @note None
  */
uint16_t Queue_Size(Queue_t *queue)
{
  if(queue->tail > queue->head)                                 // Head has wrapped around ring buf
    return queue->len - queue->tail + queue->head;
  else if(queue->tail < queue->head)                            // Head has not wrapped around
    return queue->head - queue->tail;
  else                                                          // Head and tail are equal - Either empty or full buffer
  {
    if(queue->isEmpty)
      return 0;
    else
      return queue->len;
  }
}

/**
  * @brief Checks the maximum size of the specified queue
  * @param queue Handle to a queue instance
  * @retval uint16_t Max size of queue
  * @note None
  */
uint16_t Queue_MaxSize(Queue_t *queue)
{
  return queue->len;
}
