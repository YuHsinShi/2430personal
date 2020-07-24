#include <malloc.h>
#include <string.h>
#include "ite_queue.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

QueueHandle_t ite_queue_new(int QSize)
{
    QueueHandle_t tmpQ_handler = NULL;

    tmpQ_handler = xQueueCreate(QSize, sizeof(IteQueueblk));
    if(tmpQ_handler == NULL) {
        printf("[%s] ERROR: Create queue fail\n", __FUNCTION__);
    }

    return tmpQ_handler;
}

void ite_queue_free(QueueHandle_t QHandler)
{
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return;
    }
    
    vQueueDelete(QHandler);
}

int ite_queue_get(QueueHandle_t QHandler, IteQueueblk *qblk)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    if(uxQueueMessagesWaiting(QHandler)) {
        xQueueReceive(QHandler, qblk, 0);
    }
    else {
        // Queue is Empty
        ret = -1;
    }

    return ret;
}

int ite_queue_get_fromISR(QueueHandle_t QHandler, IteQueueblk *qblk, portBASE_TYPE xHigherPriorityTaskWoken)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        ithPrintf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    if(uxQueueMessagesWaiting(QHandler)) {
        xQueueReceiveFromISR(QHandler, qblk, &xHigherPriorityTaskWoken);
    }
    else {
        // Queue is Empty
        ret = -1;
    }

    return ret;
}

int ite_queue_put(QueueHandle_t QHandler, IteQueueblk *qblk)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }
    
    // write to Queue
    if(xQueueSend(QHandler, qblk, 0) != pdPASS) {
        // Queue is FULL
        ret = -1;
    }

    return ret;
}

int ite_queue_put_fromISR(QueueHandle_t QHandler, IteQueueblk *qblk, portBASE_TYPE xHigherPriorityTaskWoken)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        ithPrintf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }
    
    // write to Queue
    if(xQueueSendFromISR(QHandler, qblk, &xHigherPriorityTaskWoken) != pdPASS) {
        // Queue is FULL
        ret = -1;
    }

    return ret;
}

int ite_queue_put_head(QueueHandle_t QHandler, IteQueueblk *qblk)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    // write to Queue
    if(xQueueSendToFront(QHandler, qblk, 0) != pdPASS) {
        // Queue is FULL
        ret = -1;
    }

    return -1;
}

int ite_queue_put_head_fromISR(QueueHandle_t QHandler, IteQueueblk *qblk, portBASE_TYPE xHigherPriorityTaskWoken)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        ithPrintf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    // write to Queue
    if(xQueueSendToFront(QHandler, qblk, &xHigherPriorityTaskWoken) != pdPASS) {
        // Queue is FULL
        ret = -1;
    }

    return -1;
}

int ite_queue_get_size(QueueHandle_t QHandler)
{
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    return uxQueueMessagesWaiting(QHandler);
}

void ite_queue_reset(QueueHandle_t QHandler)
{
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    // set queue to emtpy
    xQueueReset(QHandler);
}

int ite_queue_peek_head(QueueHandle_t QHandler, IteQueueblk *qblk)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        printf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    if(uxQueueMessagesWaiting(QHandler)) {
        xQueuePeek(QHandler, qblk, 0);
    }
    else {
        // Queue is empty
        ret = -1;
    }

    return ret;
}

int ite_queue_peek_head_fromISR(QueueHandle_t QHandler, IteQueueblk *qblk)
{
    int ret = 0;
    
    if(QHandler == NULL) {
        ithPrintf("[%s] ERROR: Queue handler is empty\n", __FUNCTION__);
        return -1;
    }

    if(uxQueueMessagesWaiting(QHandler)) {
        xQueuePeekFromISR(QHandler, qblk);
    }
    else {
        // Queue is empty
        ret = -1;
    }

    return ret;
}



