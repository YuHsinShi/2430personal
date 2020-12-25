#include "ite/itp.h"
#include "saradc/saradc.h"
#include "ncp18_sensor.h"


#define ADC_TEMPRATURE			0
#define ADC_LIGHT				1
#define ADC_PWR_CUT				2


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

void ADC_Init(void)
{
    SARADC_RESULT result = SARADC_SUCCESS;

    uint16_t      writeBuffer_len = 512;
    uint16_t      calibrationOutput = 0;
    float         NCP18calibrationOutput = 0;

    result = mmpSARInitialize(SARADC_MODE_AVG_ENABLE, SARADC_MODE_STORE_RAW_ENABLE, SARADC_AMPLIFY_1X, SARADC_CLK_DIV_9);
    if (result)
    {
        printf("mmpSARInitialize() error (0x%x) !!\n", result);
        printf("ADC_Init error\n");
    }
    else
    {
        printf("ADC_Init success\n");
    }



    if (result = mmpSARConvert(ADC_PWR_CUT, writeBuffer_len, &calibrationOutput))
    {
        printf("mmpSARConvert() error (0x%x) !!\n", result);
        while(1) sleep(1);
    }
    else
    {
			printf("ADC_PWR_CUT :Sudden Drop Voltage=%d,%f\n", calibrationOutput,result);

    }


	
}



const  unsigned char temp_rom[256]={
200,200,200,200,200,200,200,200,200,200,200,200,200,200,200,226,226,226,226,227,228,229,230,
231,232,233,234,235,235,236,236,237,238,239,240,240,241,241,242,243,243,244,244,245,246,246,
247,247,248,248,249,249,250,250,250,251,251,252,252,253,253,254,254,255,255,0,0,1,1,1,2,2,3,
3,4,4,4,5,5,6,6,6,7,7,8,8,9,9,9,10,10,11,11,11,12,12,13,13,13,14,14,15,15,15,16,16,17,17,17,
18,18,19,19,19,20,20,21,21,21,22,22,22,23,23,24,24,24,25,25,26,26,27,27,27,28,28,29,29,29,30,
30,31,31,31,32,32,33,33,34,34,35,35,35,36,36,37,37,38,38,39,39,39,40,40,41,41,42,42,43,43,44,
44,45,45,46,46,47,48,48,49,49,50,50,51,52,52,53,53,54,55,55,56,56,57,58,58,59,60,61,61,62,63,
63,64,65,66,67,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,83,84,85,86,88,90,91,93,94,95,97,
100,101,103,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,105,
};

float NCP18_Detect(void)
{
    float         result = NCP18_ERROR_T_C;
    float         temp_temp = 0;
    SARADC_RESULT result_s = SARADC_SUCCESS;
    uint16_t      writeBuffer_len = 512;
    uint16_t      calibrationOutput = 0;
    float         NCP18calibrationOutput = 0;
	float calib;
	unsigned char index;

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
//SAR ADC 0 
    if (result_s = mmpSARConvert(ADC_TEMPRATURE, writeBuffer_len, &calibrationOutput))
    {
        printf("mmpSARConvert() error (0x%x) !!\n", result_s);
        while(1) sleep(1);
    }
    else
    {


		calib=(256.*(float)calibrationOutput/4096. );

		index = (unsigned char )calib;
		if(index==0xff)
				index ==0xfe;
		result=(float)temp_rom[index];
        //printf("Calibration Output:%d,%f\n", calibrationOutput,result);

		/*

	
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
        */
    }

    return result;
}
