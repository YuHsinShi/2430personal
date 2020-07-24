#include <malloc.h>
#include <string.h>
#include "flower.h"

//=============================================================================
//                              Constant Definition
//=============================================================================
extern IteFilterDes FilterA;
extern IteFilterDes FilterB;
extern IteFilterDes FilterC;
extern IteFilterDes FilterD;
extern IteFilterDes FilterE;
extern IteFilterDes FilterF;
extern IteFilterDes FilterCapture;

// Filter Set
IteFilterDes *gFilterDesSet[] = {
    &FilterA,
    &FilterB,
    &FilterC,
    &FilterD,
    &FilterE,
    &FilterF,
    &FilterCapture,
    NULL
};



