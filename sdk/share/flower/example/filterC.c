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
            printf("[%s] FilterC data=%x\n", __FUNCTION__, blk.data);
        }
        
        //reset
        blk.data = 0;     
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

static IteMethodDes FilterC_methods[] = {
    {ITE_FILTER_C_Method, test_method},
    {0, NULL}
};

IteFilterDes FilterC = {
    ITE_FILTER_C_ID,
    test_init,
    test_uninit,
    test_preprocess,
    test_process,
    test_postprocess,
    FilterC_methods
};



