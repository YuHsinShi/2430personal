#include "ite/itp.h"
#include "saradc/saradc.h"
#include "ncp18_sensor.h"

typedef struct TV_MAPPING_ENTRY_TAG
{
    float    temperatureInput;
    float    voltageOutput;
} TV_MAPPING_ENTRY;

static TV_MAPPING_ENTRY tINTERPOLATIONMappingTable[] =
{
    {-25, 3.1038}, {-24, 3.0915}, {-23, 3.0787}, {-22, 3.0652}, {-21, 3.0510}, {-20, 3.0362}, {-19, 3.0207}, {-18, 3.0045}, {-17, 2.9875}, {-16, 2.9698},
    {-15, 2.9514}, {-14, 2.9322}, {-13, 2.9121}, {-12, 2.8913}, {-11, 2.8697}, {-10, 2.8473}, { -9, 2.8240}, { -8, 2.7999}, { -7, 2.7750}, { -6, 2.7493},
    { -5, 2.7227}, { -4, 2.6954}, { -3, 2.6672}, { -2, 2.6382}, { -1, 2.6085}, {  0, 2.5779}, {  1, 2.5466}, {  2, 2.5145}, {  3, 2.4817}, {  4, 2.4482},
    {  5, 2.4141}, {  6, 2.3793}, {  7, 2.3439}, {  8, 2.3080}, {  9, 2.2715}, { 10, 2.2345}, { 11, 2.1970}, { 12, 2.1592}, { 13, 2.1210}, { 14, 2.0824},
    { 15, 2.0436}, { 16, 2.0045}, { 17, 1.9652}, { 18, 1.9258}, { 19, 1.8863}, { 20, 1.8468}, { 21, 1.8072}, { 22, 1.7677}, { 23, 1.7283}, { 24, 1.6891},
    { 25, 1.6500}, { 26, 1.6111}, { 27, 1.5725}, { 28, 1.5342}, { 29, 1.4963}, { 30, 1.4587}, { 31, 1.4215}, { 32, 1.3848}, { 33, 1.3485}, { 34, 1.3128},
    { 35, 1.2775}, { 36, 1.2428}, { 37, 1.2087}, { 38, 1.1752}, { 39, 1.1422}, { 40, 1.1099}, { 41, 1.0782}, { 42, 1.0472}, { 43, 1.0168}, { 44, 0.9870},
    { 45, 0.9580}, { 46, 0.9295}, { 47, 0.9018}, { 48, 0.8747}, { 49, 0.8482}, { 50, 0.8224}, { 51, 0.7973}, { 52, 0.7729}, { 53, 0.7491}, { 54, 0.7259},
    { 55, 0.7034}, { 56, 0.6815}, { 57, 0.6603}, { 58, 0.6396}, { 59, 0.6195}, { 60, 0.6000}, { 61, 0.5811}, { 62, 0.5628}, { 63, 0.5450}, { 64, 0.5277},
    { 65, 0.5110}, { 66, 0.4948}, { 67, 0.4792}, { 68, 0.4640}, { 69, 0.4493}, { 70, 0.4350}, { 71, 0.4212}, { 72, 0.4078}, { 73, 0.3949}, { 74, 0.3823},
    { 75, 0.3702}, { 76, 0.3585}, { 77, 0.3472}, { 78, 0.3362}, { 79, 0.3256}, { 80, 0.3153}, { 81, 0.3054}, { 82, 0.2958}, { 83, 0.2866}, { 84, 0.2776},
    { 85, 0.2689}
};

static int NCP18_TableCalibrate(float input, float *output)
{
    int         result = 0;
    float       ratio_temp = 0;
    uint16_t    real_index = 0;

    if (input < 0 || input > 3.3 || !output)
    {
        printf("NCP18_TableCalibrate parameter fail\n");
        result = 1;
        goto end;
    }

    for (real_index = 0; real_index < (sizeof(tINTERPOLATIONMappingTable) / sizeof(tINTERPOLATIONMappingTable[0])); real_index++)
    {
        if (tINTERPOLATIONMappingTable[real_index].voltageOutput <= input)
            break;
    }

    if (real_index)
    {
        if (real_index == sizeof(tINTERPOLATIONMappingTable) / sizeof(tINTERPOLATIONMappingTable[0]))
        {
            printf("NCP18_TableCalibrate calibration fail\n");
            result = 1;
            goto end;
        }

        ratio_temp = (tINTERPOLATIONMappingTable[real_index - 1].voltageOutput - input) /
                (tINTERPOLATIONMappingTable[real_index - 1].voltageOutput - tINTERPOLATIONMappingTable[real_index].voltageOutput);

        *output = tINTERPOLATIONMappingTable[real_index - 1].temperatureInput + ratio_temp;
    }
    else
    {
        *output = tINTERPOLATIONMappingTable[real_index].temperatureInput;
    }

end:
    return result;
}

void NCP18_Init(void)
{
    SARADC_RESULT result = SARADC_SUCCESS;

    result = mmpSARInitialize(SARADC_MODE_AVG_ENABLE, SARADC_MODE_STORE_RAW_ENABLE, SARADC_AMPLIFY_1X, SARADC_CLK_DIV_9);
    if (result)
    {
        printf("mmpSARInitialize() error (0x%x) !!\n", result);
        printf("NCP18_Init error\n");
    }
    else
    {
        printf("NCP18_Init success\n");
    }
}

float NCP18_Detect(void)
{
    float         result = NCP18_ERROR_T_C;
    float         temp_temp = 0;
    SARADC_RESULT result_s = SARADC_SUCCESS;
    uint16_t      writeBuffer_len = 512;
    uint16_t      calibrationOutput = 0;
    float         NCP18calibrationOutput = 0;

    if (theConfig.ncp18_offset <= 0)
    {
        if ((theConfig.ncp18_offset * -1) >= NCP_18_TOLERANCE)
        {
            printf("NCP18_Detect offset is out of range(%f)\n", theConfig.ncp18_offset);
            return result;
        }
    }
    else
    {
        if (theConfig.ncp18_offset >= NCP_18_TOLERANCE)
        {
            printf("NCP18_Detect offset is out of range(%f)\n", theConfig.ncp18_offset);
            return result;
        }
    }

    if (result_s = mmpSARConvert(1, writeBuffer_len, &calibrationOutput))
    {
        printf("mmpSARConvert() error (0x%x) !!\n", result_s);
        while(1) sleep(1);
    }
    else
    {
        //printf("Calibration Output:%d\n", calibrationOutput);
        temp_temp = (float)calibrationOutput;
        temp_temp = temp_temp / 4095 * 3.3;

        if (!NCP18_TableCalibrate(temp_temp, &NCP18calibrationOutput))
        {
            NCP18calibrationOutput = NCP18calibrationOutput + theConfig.ncp18_offset;
            result = NCP18calibrationOutput;

            if (result < 0) result = 0;
            else if (result > 99) result = 99;
        }
    }

    return result;
}
