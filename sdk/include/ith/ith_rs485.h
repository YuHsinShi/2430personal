#ifndef ITH_RS485_H
#define ITH_RS485_H

/** @addtogroup ith ITE Hardware Library
 *  @{
 */
/** @addtogroup ith_rs485 RS485
 *  @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RS485 port definition.
 */
typedef enum
{
    ITH_RS485_0 = 0,     ///< RS485 port #0
    ITH_RS485_1 = 1,     ///< RS485 port #1  UART1 using for debug , so RS485 didn`t use it.
    ITH_RS485_2 = 2,     ///< RS485 port #2
    ITH_RS485_3 = 3,     ///< RS485 port #3
    ITH_RS485_4 = 4,     ///< RS485 port #4
    ITH_RS485_5 = 5,     ///< RS485 port #5
} ITHRS485Port;

#ifdef __cplusplus
}
#endif

#endif // ITH_RS485_H
/** @} */ // end of ith_rs485
/** @} */ // end of ith