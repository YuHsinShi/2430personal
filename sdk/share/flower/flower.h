/*
 * Copyright (c) 2004 ITE Technology Corp. All Rights Reserved.
 */
/** @file
 * Flower.
 *
 * @author
 * @version 1.0
 */
#ifndef FLOWER_H
#define FLOWER_H

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <semaphore.h>
#include <stdbool.h>
#include <pthread.h>
#include "ite_queue.h"

#define DEBUG_PRINT(...)
//#define DEBUG_PRINT     printf
#define STREAM_QUEUE_SIZE      32

typedef void (*FilterFunc)(struct _IteFilter *f);
typedef void (*MethodFunc)(struct _IteFilter *f, void *arg);

//=============================================================================
//                              Structure Definition
//=============================================================================

/**
 * Filter ID
 */
typedef enum _IteFilterId {
    ITE_FILTER_A_ID = 1,
    ITE_FILTER_B_ID,
    ITE_FILTER_C_ID,
    ITE_FILTER_D_ID,
    ITE_FILTER_E_ID,
    ITE_FILTER_F_ID,
    ITE_FILTER_CAP_ID,
} IteFilterId;

/**
 * Method fucntion ID
 */
typedef enum _IteMethodId {
    ITE_FILTER_A_Method = 1,
    ITE_FILTER_B_Method,
    ITE_FILTER_C_Method,
    ITE_FILTER_D_Method,
    ITE_FILTER_E_Method,
    ITE_FILTER_F_Method,
    ITE_FILTER_CAP_SetWidth,
    ITE_FILTER_CAP_SetHeight,
}IteMethodId;

/**
 * Method fucntion description
 */
struct _IteMethodDes {
    IteMethodId id;
    MethodFunc method_func;
};
typedef struct _IteMethodDes IteMethodDes;

/**
 * Filter function description
 */
struct _IteFilterDes {
    IteFilterId id;
    FilterFunc init;
    FilterFunc uninit;
    FilterFunc preprocess;
    FilterFunc process;
    FilterFunc postprocess;
    IteMethodDes *method;
};
typedef struct _IteFilterDes IteFilterDes;

/**
 * Filter's input & output data structure
 */
struct _IteFilterParm {
    QueueHandle_t Qhandle;
    sem_t semHandle;
};
typedef struct _IteFilterParm IteFilterParm;

/**
 * Filter definition
 */
struct _IteFilter {
    IteFilterParm input[2];
    IteFilterParm output[2];
    IteFilterDes filterDes;
    pthread_t tID;
    bool run;
    bool inputSemBind;
};
typedef struct _IteFilter IteFilter;

/**
 * Filter chain list
 */
struct _IteFChain {
    IteFilter *filter;
    struct _IteFChain *nextPtr;
};
typedef struct _IteFChain IteFChain;

/**
 * Filter chain's configure
 */
struct _IteFcConf {
    char *name;
    bool Semaphore;
    int QSize;
};
typedef struct _IteFcConf IteFcConf;

/**
 * Flow list definition
 */
struct _IteFlowerList {
    IteFChain *fc;
    IteFcConf config;
};
typedef struct _IteFlowerList IteFlowerList;

//=============================================================================
//                              Global Data Definition
//=============================================================================

extern IteFilterDes *gFilterDesSet[];


//=============================================================================
//                              Public Function Definition
//=============================================================================

/**
 * Malloc a new filter.
 *
 * @param id The new filter ID
 * @return Filter
 */
IteFilter *ite_filter_new(IteFilterId id);

/**
 * Free a filter
 *
 * @param f The filter you want to free
 */
void ite_filter_delete(IteFilter *f);

/**
 * Call filter's method function
 *
 * @param f The filter
 * @param MId The method function ID
 * @param arg The method function's argument
 */
void ite_filter_call_method(IteFilter *f, int MId, void *arg);

/**
 * Sets when all input of filter use the same semaphore
 *
 * @param f The filter
 */
void ite_filter_set_semBind(IteFilter *f);

/**
 * Sets filter chain configure
 *
 * @param fc The filter chain
 * @param param_size The number of configure parameter
 * @param param The configure parameter
 */
void ite_filterChain_setConfig(IteFChain *fc, int param_size, char **param);

/**
 * Build the filter chain
 *
 * @param helper The filter chain
 * @param name The filter chain name
 */
void ite_filterChain_build(IteFChain *helper, char *name);

/**
 * Print all filter in the filter chain
 *
 * @param helper The filter chain
 */
void ite_filterChain_print(IteFChain *helper);

/**
 * Stop all threads of filter chain
 *
 * @param helper The filter chain
 */
void ite_filterChain_stop(IteFChain *helper);

/**
 * Delete the filter chain
 *
 * @param helper The filter chain
 */
void ite_filterChain_delete(IteFChain *helper);

/**
 * Link filter to filter chain
 *
 * @param helper the filter chain
 * @param preFoutPin The output Pin number of filter in filter chain
 * @param f The filter you want to link
 * @param inputPin The input Pin number of filter you want to add to link
 */
void ite_filterChain_link(IteFChain *helper, int preFoutPin, IteFilter *f, int inputPin);

/**
 * Unlink filter from filter chain
 *
 * @param helper the filter chain
 * @param preFoutPin The output Pin number of filter
 * @param f The filter you want to unlink
 * @param inputPin The input Pin number of filter you want to unlink
 */
void ite_filterChain_unlink(IteFChain *helper, int preFoutPin, IteFilter *f, int inputPin);

/**
 * Create all threads in filter chain to run
 *
 * @param helper The filter chain
 */
void ite_filterChain_run(IteFChain *helper);

/**
 * Init flower list
 *
 */
void ite_flower_init(void);

/**
 * Reset flower list
 *
 */
void ite_flower_reset(void);


/**
 * Add a filter chain in flower list
 *
 * @param fc The filter chain
 * @param name The filter chain name
 */
void ite_flower_add(IteFChain *fc, char *name);

/**
 * Get queue size of filter chain
 *
 * @param fc The filter chain
 * @return Queue size
 */
int ite_flower_findFChainQSize(IteFChain *fc);

/**
 * Get semaphore option of filter chain
 *
 * @param fc The filter chain
 * @return if use semaphore or not
 */
bool ite_flower_findFChainUseSem(IteFChain *fc);

/**
 * Get configure of filter chain
 *
 * @param fc The filter chain
 * @return The filter chain configure
 */
IteFcConf *ite_flower_findFChainConfig(IteFChain *fc);

/**
 * Print all filter chains in flower list
 *
 * @param f The filter
 */
void ite_flower_print(void);

/**
 * Remove a filter chain from the flower list
 *
 * @param fc The filter chain
 */
void ite_flower_delete(IteFChain *fc);

/**
 * Deinit flower list
 *
 */
void ite_flower_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /* ITE_STREAMER_H */
