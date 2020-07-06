/********************************************************************************
  * @file       : list.h
  * @brief      : Header file for a generic light-weight linked-list library to build a dynamic list.
  *               See the comments below for example implementation.
  * @author     : Charles H
  * @date       : Mar 2020
  *
  *******************************************************************************/
#ifndef __LIST_H
#define __LIST_H

#include <stdint.h>
#include <stdbool.h>

/**
  * @brief This is a struct to each node in the linked list.
            It contains the pointer to the next node and a pointer to the object it holds
  */
typedef struct ListNode
{
    struct ListNode *next;
    void *object;
} ListNode_t;

/**
  * @brief This is a handle to an instance of the linked list. It contains a pointer to the first node in the list, as well as properties regarding
            size of the objects it holds and if it's empty.
  */
typedef struct ListHandle
{
    ListNode_t *list_head;
    uint16_t object_size;
    bool empty;
} ListHandle_t;

/**
  * @brief List status return values for functions
  */
typedef enum ListStatus
{
    LIST_OK,
    LIST_FULL,
    LIST_FAILED
} ListStatus_t;

/* Function Declarations */
ListStatus_t List_Init(ListHandle_t *hlist, uint16_t object_size);
ListStatus_t List_Add_To_End(ListHandle_t *hlist, void *value);
ListStatus_t List_Add_To_Beginning(ListHandle_t *hlist, void *value);
bool List_Is_Empty(ListHandle_t *hlist);
//ListStatus_t List_Remove(ListHandle_t *hlist, uint16_t index);
#endif //__LIST_H