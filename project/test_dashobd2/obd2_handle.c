//****************************
/*TEST OBD2
/*VERSION  V0.1
/*USE MODULE
/*(1)MCP2517FD click board
/*(2)OBD2 car ecu simulator 
*****************************/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <malloc.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "obd2.h"
#include "obd2_handle.h"

/******************************************/
/*                        LOCAL DEFINE                      */
/*****************************************/
int CAR_SPEED;
int CAR_RPM;
int CAR_FUEL_LEVEL;
int CAR_COOLANT_TEMP;

/******************************************/
/*                        LOCAL STRUCT                     */
/*****************************************/

/******************************************/
/*                        LOCAL VALUE                       */
/*****************************************/

/******************************************/
/*                        LOCAL FUN                          */
/*****************************************/
void* OBD2_HANDLETASK(void* arg)
{
    //OBD_DTC_NODE* obd_p = NULL;
    /*OBD2 protocol init*/
    OBD2Init(PRO_CAN_11B_500K);
    while(1)
    {

        if(OBD2ReadPID(PID_SPEED, &CAR_SPEED) != 0)
            CAR_SPEED = 0;
        //printf("SPEED = %d \n",CAR_SPEED);
        if(OBD2ReadPID(PID_RPM, &CAR_RPM) != 0)
            CAR_RPM = 0;
        
        //printf("RPM = %d \n",CAR_RPM);
        if(OBD2ReadPID(PID_FUEL_LEVEL, &CAR_FUEL_LEVEL) != 0)
            CAR_FUEL_LEVEL = 0;
        //printf("FUEL_LEVEL = %d %% \n",CAR_FUEL_LEVEL);
        
        if(OBD2ReadPID(PID_COOLANT_TEMP, &CAR_COOLANT_TEMP) != 0)
            CAR_COOLANT_TEMP = 0;
        //printf("COOLANT = %d %% \n",CAR_COOLANT_TEMP);       

#if 0
        obd_p = (OBD_DTC_NODE*)malloc(sizeof(OBD_DTC_NODE) * 10);

        OBD2ReadDTC(obd_p);

        for(i  = 0 ; i < obd_p[0].num ; i++)
        {
            printf("type: %c %02d %02d\n",obd_p[i].system ,obd_p[i].dtc_h,  obd_p[i].dtc_l);
        }
        free(obd_p);
#endif
        usleep(1000);
    }
 
}

int OBD2_GETSPEED(void)
{
    return CAR_SPEED;
}

int OBD2_GETRPM(void)
{
    return CAR_RPM;
}

int OBD2_GETFUEL(void)
{
    return CAR_FUEL_LEVEL;
}

int OBD2_GETCOOLANTTEMP(void)
{
    return CAR_COOLANT_TEMP;
}

void OBD2TASKInit()
{
    pthread_t task;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
	pthread_create(&task, &attr, OBD2_HANDLETASK, NULL);

}

