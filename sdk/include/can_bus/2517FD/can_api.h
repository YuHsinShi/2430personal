#ifndef CAN_INTERFACE_H
#define CAN_INTERFACE_H

#include "ite/ith.h"
#include "ite/itp.h"
#include "ssp/mmp_spi.h"
#include "Drv_spi.h"
#include "drv_canfdspi_defines.h"
#include "drv_canfdspi_api.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility
extern "C" {
#endif
// DOM-IGNORE-END

// Use RX and TX Interrupt pins to check FIFO status
//#define APP_USE_RX_INT

//Use timestamp
//#define APP_USE_TIME_STAMP

#define MAX_TXQUEUE_ATTEMPTS 50

// Interrupts GPIO
//#define GEN_INT ITH_GPIO_PIN31
//#define TX_INT  ITH_GPIO_PIN30
//#define RX_INT  ITH_GPIO_PIN29
//#define CAN_STBY ITH_GPIO_PIN36

// Message IDs
#define TX_REQUEST_ID       0x300
#define TX_RESPONSE_ID      0x301
//#define BUTTON_STATUS_ID    0x201
//#define LED_STATUS_ID       0x200
//#define PAYLOAD_ID          0x101

// Transmit Channels
#define APP_TX_FIFO CAN_FIFO_CH2

// Receive Channels
#define APP_RX_FIFO CAN_FIFO_CH1

// Switch states

#if 0
typedef struct {
	bool S1;
} APP_SwitchState;

// Payload

typedef struct {
	bool On;
	uint8_t Dlc;
	bool Mode;
	uint8_t Counter;
	uint8_t Delay;
	bool BRS;
} APP_Payload;
#endif
typedef enum {
    // Initialization
    APP_STATE_INIT = 0,
    APP_STATE_INIT_TXOBJ,

    // POR signaling
    APP_STATE_FLASH_LEDS,

    // Transmit and Receive
    APP_STATE_TRANSMIT,
    APP_STATE_RECEIVE,
    APP_STATE_PAYLOAD,

    // Test SPI access
    APP_STATE_TEST_RAM_ACCESS,
    APP_STATE_TEST_REGISTER_ACCESS,

    // Switch monitoring
    APP_STATE_SWITCH_CHANGED,
    APP_STATE_TSET_SLEEP,
} APP_STATES;

typedef struct {
    /* The application's current state */
    APP_STATES state;

    /* TODO: Define any additional data used by the application. */


} APP_DATA;

void ithCANOpen(uint8_t CANFDSPI_INDEX, CAN_BITTIME_SETUP bitTime, CAN_SYSCLK_SPEED clk);

//! Application Tasks
// This routine must be called from SYS_Tasks() routine.

//void APP_Tasks(void);

//! Write LEDs based on input byte

//void APP_LED_Write(uint8_t led);

//! Clear One LED

//void APP_LED_Clear(uint8_t led);

//! Set One LED

//void APP_LED_Set(uint8_t led);


//! Add message to transmit FIFO
//void APP_TransmitMessageQueue(void);
void ithCANWrite(uint8_t CANFDSPI_INDEX ,CAN_TX_MSGOBJ* txObj, uint8_t *txd);

//! Decode received messages
//APP_STATES APP_ReceiveMessage_Tasks(void);
bool ithCANRead(uint8_t CANFDSPI_INDEX, CAN_RX_MSGOBJ* rxObj, uint8_t *rxd);

//! Transmit switch state
//void APP_TransmitSwitchState(void);

//! Periodically send message with requested payload
//void APP_PayLoad_Tasks(void);

//! Test SPI access
//bool ithCAN_TestRegisterAccess(void);

//! Test RAM access
bool ithCANTestRamAccess(uint8_t CANFDSPI_INDEX);
bool ithCANSleepMode(uint8_t CANFDSPI_INDEX);
uint32_t ithCANDlcToDataBytes(CAN_DLC dlc);
CAN_DLC ithCANDataBytesToDlc(uint8_t n);

#ifdef __cplusplus
}
#endif

#endif // CAN_INTERFACE_H