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
    blk.data = 0x1;
    
    while(1) {
        if(ite_queue_put(f->output[0].Qhandle, &blk) == 0) {
            printf("[%s] FilterA put data\n", __FUNCTION__);
            
            sem_post(&f->output[0].semHandle);
        }
        sleep(3);
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

static IteMethodDes FilterA_methods[] = {
    {ITE_FILTER_A_Method, test_method},
    {0, NULL}
};

IteFilterDes FilterA = {
    ITE_FILTER_A_ID,
    test_init,
    test_uninit,
    test_preprocess,
    test_process,
    test_postprocess,
    FilterA_methods
};



