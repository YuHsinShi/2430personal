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
    printf("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
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

}

static void test_postprocess(IteFilter *f)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static void test_method(IteFilter *f, void *arg)
{
    DEBUG_PRINT("[%s] Filter(%d)\n", __FUNCTION__, f->filterDes.id);
}

static IteMethodDes FilterE_methods[] = {
    {ITE_FILTER_E_Method, test_method},
    {0, NULL}
};

IteFilterDes FilterE = {
    ITE_FILTER_E_ID,
    test_init,
    test_uninit,
    test_preprocess,
    test_process,
    test_postprocess,
    FilterE_methods
};



