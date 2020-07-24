#include <stdio.h>
#include "flower.h"

//=============================================================================
//                              Constant Definition
//=============================================================================

//=============================================================================
//                              Private Function Declaration
//=============================================================================

static void test_init(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void test_uninit(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void test_preprocess(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void test_process(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);

#if 1
    while(1) {
        printf("[%s] Filter(%d). thread run\n", __FUNCTION__, f->filterDes.id);
        sleep(3);
    }
#endif


#if 0
    IteQueueblk blk;
    while(1) {
        sem_wait(&f->input[0].semHandle);

        if(ite_queue_get(f->input[0].Qhandle, &blk) == 0) {
            blk.data = blk.data | 0x2;
            
            sem_post(&f->output[0].semHandle);
        }

        blk.data = 0;
    }
#endif

#if 0
    IteQueueblk blk0, blk1;
    while(1) {               
        sem_wait(&f->input[0].semHandle);
        
        if(ite_queue_get(f->input[0].Qhandle, &blk0) == 0) {        
            blk0.data = blk0.data | 0x2;
            
            if(ite_queue_put(f->output[0].Qhandle, &blk0) == 0) {
                sem_post(&f->output[0].semHandle);
            }

            printf("[%s] FilterB blk0=%x\n", __FUNCTION__, blk0.data);
        }
        
        sem_wait(&f->input[1].semHandle);

        if(ite_queue_get(f->input[1].Qhandle, &blk1) == 0) {
            blk1.data = blk0.data | blk1.data;
            
            if(ite_queue_put(f->output[1].Qhandle, &blk1) == 0) {
                sem_post(&f->output[1].semHandle);
            }

            printf("[%s] FilterB blk1=%x\n", __FUNCTION__, blk1.data);
        }

        //reset
        blk0.data = 0;
        blk1.data = 0;
    }
#endif
}

static void test_postprocess(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void test_method(IteFilter *f, void *arg)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static IteMethodDes FilterB_methods[] = {
    {ITE_FILTER_B_Method, test_method},
    {0, NULL}
};


IteFilterDes FilterB = {
    ITE_FILTER_B_ID,
    test_init,
    test_uninit,
    test_preprocess,
    test_process,
    test_postprocess,
    FilterB_methods
};



