#ifndef OBD2_HANDLE_H
#define OBD2_HANDLE_H

#ifdef __cplusplus
extern "C" {
#endif

/******************************************/
/*                        LOCAL DEFINE                      */
/*****************************************/

/******************************************/
/*                        LOCAL STRUCT                     */
/*****************************************/

/******************************************/
/*                        LOCAL VALUE                       */
/*****************************************/

/******************************************/
/*                        LOCAL FUN                          */
/*****************************************/
void* OBD2_HANDLETASK(void* arg);
void OBD2TASKInit();
int OBD2_GETSPEED(void);
int OBD2_GETRPM(void);
int OBD2_GETFUEL(void);
int OBD2_GETCOOLANTTEMP(void);

#ifdef __cplusplus
}
#endif

#endif

