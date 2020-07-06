/********************************************************************************
  * @file       : list.c
  * @brief      : This is a generic light-weight linked-list library to build a dynamic list.
  *               See the comments below for example implementation.
  * @author     : Charles H
  * @date       : Mar 2020
  *
  *******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include "list.h"

/**
  * @brief This initializes the first node and sets up the list for usage
  * @param hlist Handle to the list instance
  * @param object_size Size of objects (bytes) put into each node
  * @retval None
  */
ListStatus_t List_Init(ListHandle_t *hlist, uint16_t object_size)
{
    // Malloc appropriate memory for first list_node
    hlist->list_head = (void *)malloc(sizeof(ListNode_t));
    if(hlist->list_head == NULL)
      return LIST_FAILED;

    hlist->list_head->next = NULL; // End of list

    // Set other parameters
    hlist->object_size = object_size;
    hlist->empty = true;

    return LIST_OK;
}

/**
  * @brief Adds the value to the end of the list. Loops through all the nodes and adds on to the end
  * @param value Pointer to the data to store in the list. Uses the ListHandle_t->object_size to determine how many bytes to copy
  * @retval None
  */
ListStatus_t List_Add_To_End(ListHandle_t *hlist, void *value)
{
    if (hlist->empty)
    {
        hlist->list_head->object = (void*)malloc(hlist->object_size); // Allocate object only
        if(hlist->list_head->object == NULL)
          return LIST_FAILED;

        hlist->list_head->next = NULL; // End of list
        hlist->empty = false;
        memcpy(hlist->list_head->object, value, hlist->object_size); // Copy object into memory
    }
    else
    {
        ListNode_t *node = (ListNode_t*) malloc(sizeof(ListNode_t));   // Allocate new list node
        if(node == NULL)
          return LIST_FAILED;

        node->object = (void*)malloc(hlist->object_size);       // Allocate object
        if(node == NULL)
        {
          free(node);
          return LIST_FAILED;
        }

        memcpy(node->object, value, hlist->object_size); // Copy object into memory
        node->next = NULL; // End of list

        ListNode_t *tail = hlist->list_head; // Get the tail
        while (tail->next != NULL)
        {
            tail = tail->next;
        }

        tail->next = node; // Append on to the end
    }

    return LIST_OK;
}

/**
  * @brief Adds the value to the beginning of the list. Shuffles the pointers to put the new node up front.
  * @param value Pointer to the data to store in the list. Uses the ListHandle_t->object_size to determine how many bytes to copy
  * @retval None
  */
ListStatus_t List_Add_To_Beginning(ListHandle_t *hlist, void *value)
{
    if (hlist->empty)
    {
        hlist->list_head->object = (void*)malloc(hlist->object_size); // Allocate object only
        if(hlist->list_head->object == NULL)
          return LIST_FAILED;

        hlist->empty = false;
        memcpy(hlist->list_head->object, value, hlist->object_size); // Copy object into memory
        hlist->list_head->next = NULL; // End of list
    }
    else
    {
        ListNode_t *node = (ListNode_t*)malloc(sizeof(ListNode_t));   // Allocate new list node
        if(node == NULL)
          return LIST_FAILED;

        node->object = (void*)malloc(hlist->object_size);       // Allocate object
        if(node->object == NULL)
        {
          free(node);
          return LIST_FAILED;
        }
        memcpy(node->object, value, hlist->object_size); // Copy object into memory
        node->next = hlist->list_head;                   // Put the temp *node as the head
        hlist->list_head = node;
    }

    return LIST_OK;
}

/**
  * @brief Checks if the list is empty
  * @param hlist Handle to the list instance
  * @retval bool Is the list empty
  */
bool List_Is_Empty(ListHandle_t *hlist)
{
  return hlist->empty;
}

/**
  * @brief
  * @param
  * @retval
  *
ListStatus_t List_Remove(ListHandle_t *hlist, uint16_t index)
{ /*
    ListNode_t* prev = hlist->list_head;
    while(prev != NULL){
        if(prev == node)    // Comparing memory pointers
    }

    while(index-- > 0){

    }*

    // This needs to automatically find the index and then remove the middle value

  return LIST_OK;
}
*/