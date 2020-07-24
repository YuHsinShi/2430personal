#include "obd2.h"
#include "obd2_hal.h"

uint8_t rxb[CAN_MAX_DATA_BYTES];
uint8_t txb[CAN_MAX_DATA_BYTES];
uint8_t PIDMAP[OBD_PIDMAP_SIZE];

// Current protocol type
static OBD_PROTOCOLS current_pt;

static pthread_mutex_t obd2_mutex = PTHREAD_MUTEX_INITIALIZER;

//==============================================

/* HEX to int */
uint32_t hex2uint32(const uint8_t *p);

/* Read PID MAP */
void ReadPIDMAP();

/* Query PID */
void SendQuery(OBD_Services sid,uint8_t pid);

/* Normalize row data */
int NormalizeData(uint8_t pid, uint8_t* data);

//==============================================
/* HEX to int */
inline uint32_t hex2uint32(const uint8_t *p)
{
	uint8_t orignal = *p;
    uint8_t low = orignal & 0xF;
    uint8_t height = orignal & 0xF0;
	uint32_t r = (height >> 4) * 16 + low;
	return r;
}

/* Read Supported PID */
void ReadPIDMAP()
{
    uint8_t i =0;
    uint8_t j =0;
    uint8_t k = 0;
    uint32_t mapindex =0;
    uint32_t timecount = 0;
    
    /* Reset PIDMAP array */
    memset(PIDMAP, 0x0, sizeof(PIDMAP));
    
    /* Scan pid 0x0  0x20 0x40...etc */
    for(i = 0; i < 3; i++)
    {
        txb[0]= 0x2;
        txb[1]= 0x1;
        txb[2]= i*0x20;
        HAL_CANWrite(txb);
        timecount = 60;
        for(;;)
        {
            if(HAL_CANRead(rxb))
            {
                for(j = 3; j < OBD_MAX_DATA_BYTES - 1; j++)
                {
                    PIDMAP[mapindex++] = rxb[j];
                }
                printf("get pid[%x] respond success\n", i*0x20);
                break;
            }
            timecount -- ;
            if(timecount <= 0)
            {
                printf("wait pid[%x] respond timeout\n", i*0x20);
                break;
            }
        }
        /* wait ecu accept next command */
        WAIT_ECU();
        
    }
#if 0/* Show Supported PID */
    for(k = 0; k < OBD_PIDMAP_SIZE; k ++)
    {
        printf("[%d] = %x\n",k,PIDMAP[k]);
    }
#endif
}

/* Send Query Command */
void SendQuery(OBD_Services sid,uint8_t pid)
{
    if(sid == RequestTruoubleCode)
        txb[0]= 0x1;
    else
        txb[0]= 0x2;
    
    txb[1]= sid;
    txb[2]= pid;
    txb[3]= 0xff;
    txb[4]= 0xff;
    txb[5]= 0xff;
    txb[6]= 0xff;
    txb[7]= 0xff;
    HAL_CANWrite(txb);
}

/* Normalize row data */
int NormalizeData(uint8_t pid, uint8_t* data)
{
	int result;
	switch (pid) {
	case PID_RPM:
	case PID_EVAP_SYS_VAPOR_PRESSURE: // kPa
		result = (int)((hex2uint32(data) * 256 + hex2uint32(data + 1)) / 4);
		break;
	case PID_FUEL_PRESSURE: // kPa
		result = (int)(hex2uint32(data) * 3);
		break;
	case PID_COOLANT_TEMP:
	case PID_INTAKE_TEMP:
	case PID_AMBIENT_TEMP:
		result = (int)(hex2uint32(data) - 40);
		break;
	case PID_THROTTLE:
	case PID_COMMANDED_EGR:
	case PID_COMMANDED_EVAPORATIVE_PURGE:
	case PID_FUEL_LEVEL:
	case PID_RELATIVE_THROTTLE_POS:
	case PID_ABSOLUTE_THROTTLE_POS_B:
	case PID_ABSOLUTE_THROTTLE_POS_C:
	case PID_ACC_PEDAL_POS_D:
	case PID_ACC_PEDAL_POS_E:
	case PID_ACC_PEDAL_POS_F:
	case PID_COMMANDED_THROTTLE_ACTUATOR:
	case PID_ENGINE_LOAD:
	case PID_ABSOLUTE_ENGINE_LOAD:
	case PID_ETHANOL_FUEL:
		result = (int)(hex2uint32(data)* 100 / 255);
		break;
	case PID_MAF_FLOW: // grams/sec
		result = (int)((hex2uint32(data)*256 + hex2uint32(data+1)) / 100);
		break;
	case PID_TIMING_ADVANCE:
		result = (int)((hex2uint32(data) / 2) - 64);
		break;
	case PID_DISTANCE: // km
	case PID_DISTANCE_WITH_MIL: // km
	case PID_TIME_WITH_MIL: // minute
	case PID_TIME_SINCE_CODES_CLEARED: // minute
	case PID_RUNTIME: // second
	case PID_FUEL_RAIL_PRESSURE: // kPa
	case PID_CONTROL_MODULE_VOLTAGE: // V
		result = (int)((hex2uint32(data)*256 + hex2uint32(data+1)) / 1000);
		break;
	case PID_SHORT_TERM_FUEL_TRIM_1:
	case PID_LONG_TERM_FUEL_TRIM_1:
	case PID_SHORT_TERM_FUEL_TRIM_2:
	case PID_LONG_TERM_FUEL_TRIM_2:
	case PID_EGR_ERROR:
		result = ((int)hex2uint32(data) - 128) * 100 / 128;
		break;
	case PID_CATALYST_TEMP_B1S1:
	case PID_CATALYST_TEMP_B2S1:
	case PID_CATALYST_TEMP_B1S2:
	case PID_CATALYST_TEMP_B2S2:
		result = (int)(hex2uint32(data)*256 + hex2uint32(data+1)) / 10 - 40;
		break;
	case PID_AIR_FUEL_EQUIV_RATIO: // 0~200
		result = (int)(hex2uint32(data)*256 + hex2uint32(data+1)) /32768;
		break;
    case PID_SPEED:
	default:
		result = hex2uint32(data);
		break;
	}
	return result;

}

/* Normalize Code */
void NormalizeCode(OBD_DTC_NODE* dtc_ptr, uint8_t* data)
{
    int i = 0;
    uint32_t size;
    uint8_t  ststem_type;

    /* get total trouble code */
    size = hex2uint32(data);
    /* max trouble code 2 */
    if(size >= 2) size = 2;
    data++;

    dtc_ptr[0].num = size;

    for(i = 0; i < size; i++)
    {
        ststem_type = (*data & 0xF0) >> 4;
        
        if(ststem_type >= 0 && ststem_type <=3 )
            dtc_ptr[i].system = 'P';
        else if(ststem_type >= 4 && ststem_type <=7 )
            dtc_ptr[i].system = 'C';
        else if(ststem_type >= 8 && ststem_type <=11 )
            dtc_ptr[i].system = 'B';
        else
            dtc_ptr[i].system = 'U';
        
        dtc_ptr[i].dtc_h = (*data & 0xF);
        data++;
        dtc_ptr[i].dtc_l = hex2uint32(data);
        data++;
    }

}

/* OBD2 init */
void OBD2Init(OBD_PROTOCOLS pt)
{
    pthread_mutex_lock(&obd2_mutex);
    uint32_t i =0;
    
    /* Init message data */
    for (i = 0; i < CAN_MAX_DATA_BYTES; i++) txb[i] = 0xff;
    HAL_CANInit(pt);
    current_pt = pt;
    ReadPIDMAP();
    pthread_mutex_unlock(&obd2_mutex);
}

/* OBD2 Service 1 Get Current Powertrain Data */
bool OBD2ReadPID(uint8_t pid, int* result)
{
    uint32_t i = 0;
    uint32_t timecount = 60;
    bool     readerror = true;
    pthread_mutex_lock(&obd2_mutex);

    SendQuery(RequestPowertrainData, pid);
    
    usleep(100);
    do
    {
        if(HAL_CANRead(rxb))
        {
            /* check respond */
            if(rxb[1] == 0x41 && rxb[2] == pid)
            {
                *result = NormalizeData(pid, &rxb[3]);
                readerror = false;
                break;
            }
        }
        timecount -- ;
    }while(timecount > 0);
    
    /* wait ecu accept next command */
    WAIT_ECU();
    pthread_mutex_unlock(&obd2_mutex);
    return readerror;
}

/* OBD2 Service 3 Get Diagnostic Trouble Codes */
/* @param[in] head of dtc array                       */
bool OBD2ReadDTC(OBD_DTC_NODE* dtc_ptr)
{
    uint32_t i = 0;
    uint32_t timecount = 60;
    bool     readerror = true;
    
    pthread_mutex_lock(&obd2_mutex);

    SendQuery(RequestTruoubleCode, 0xff);
    
    usleep(100);
    do
    {
        if(HAL_CANRead(rxb))
        {   
            /* check respond */
            if(rxb[1] == 0x43)
            {
                NormalizeCode(dtc_ptr, &rxb[2]);
                readerror = false;
                break;
            }
        }
        timecount -- ;
    }while(timecount > 0);

    /* wait ecu accept next command */
    WAIT_ECU();
    pthread_mutex_unlock(&obd2_mutex);
    return readerror;
}


