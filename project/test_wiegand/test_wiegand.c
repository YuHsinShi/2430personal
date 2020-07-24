#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "ith/ith_defs.h"

#define WIEGAND_BASE        0xDE300000
#define ITH_UART2_BASE      0xDE600200
#define ITH_UART3_BASE      0xDE600300

#define WM0                 0x000
#define WM1                 0x010
#define WSR0                0x004
#define WSR1                0x014
#define WUSR0               0x008
#define WUSR1               0x018
#define WIR0                0x00C

#define WIEGAND_RST_ENABLE  (1 << 31)
#define WIEGAND_RST_DISABLE (0 << 31)
#define WIEGAND_RESET_MASK  (1 << 31)
#define WIEGAND_D0_SEL_MASK 0x7F
#define WIEGAND_D1_SEL_MASK (0x7F << 8)

typedef enum WIEGANDID_TAG
{
    wiegand_0,
    wiegand_1,
} WIEGANDID;

typedef struct WIEGAND_OBJECT_TAG
{
    WIEGANDID id;
} WIEGAND_OBJECT;

static WIEGAND_OBJECT wgObject[2] = {
    {
     0,    //id
},
    {
     1,    //id
},
};

static void _wiegand_reset(WIEGANDID id)
{
    ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, WIEGAND_RST_ENABLE, WIEGAND_RESET_MASK);
    usleep(1000);
    ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, WIEGAND_RST_DISABLE, WIEGAND_RESET_MASK);
}

static void _wiegand_set_pins(WIEGANDID id, int d0pin, int d1pin)
{
    ithWriteRegMaskA(   WIEGAND_BASE + id * 0x010,  d0pin,      WIEGAND_D0_SEL_MASK);
    ithWriteRegMaskA(   WIEGAND_BASE + id * 0x010,  d1pin << 8, WIEGAND_D1_SEL_MASK);
}

static void _wiegand_controller_enable(WIEGANDID id, int enable)
{
    if (enable)
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (1 << 30), (1 << 30));
    else
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (0 << 30), (1 << 30));
}

static void _wiegand_parity_enable(WIEGANDID id, int enable)
{
    if (enable)
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (1 << 29), (1 << 29));
    else
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (0 << 29), (1 << 29));
}

static void _wiegand_set_polarity(WIEGANDID id, int polarity)
{
    if (polarity)
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (1 << 28), (1 << 28));
    else
        ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (0 << 28), (1 << 28));
}

static void _wiegand_set_prescale(WIEGANDID id, int value)
{
    ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id * 0x010, (value << 16), (0x1FFF << 16));
}

static void _wiegand_set_timeout(WIEGANDID id, int value)
{
    ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id * 0x010, value, 0x1FFF);
}

static void _wiegand_set_debounce(WIEGANDID id, int value)
{
    ithWriteRegMaskA(WIEGAND_BASE + WSR0 + id * 0x010, (value << 30), (0x3 << 30));
}

static void _wiegand_set_uartdiv(WIEGANDID id, int value)
{
    ithWriteRegMaskA(WIEGAND_BASE + WUSR0 + id * 0x010, value, 0xFFFF);
}

static void _wiegand_set_uartprescale(WIEGANDID id, int value)
{
    ithWriteRegMaskA(WIEGAND_BASE + WUSR0 + id * 0x010, (value << 16), (0x1F << 16));
}

static void _wiegand_set_intr_clear(WIEGANDID id, int mode)
{
    if (mode)
        ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id * 0x010, (1 << 31), (1 << 31));
    else
        ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id * 0x010, (0 << 31), (1 << 31));
}

static void _wiegand_set_probsel(WIEGANDID id, int mode)
{
    ithWriteRegMaskA(WIEGAND_BASE + WIR0 + id * 0x010, (mode << 6), (0x7 << 6));
}

static void _wiegand_set_mode(WIEGANDID id, int mode)
{
    ithWriteRegMaskA(WIEGAND_BASE + id * 0x010, (mode << 16), (0x7 << 16));
}

static void _wiegand_set_uart2(void)
{
    //uart2 Rx Source from wg0
    //ithWriteRegMaskA(ITH_GPIO_BASE+0x1A0, (1 << 30), (1 << 30));
/*
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0080);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_IIR_REG, 0x0001);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_THR_REG, 0x0008);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, 0x0002);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_IIR_TXFIFOFULL, 0x000d);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_LCR_REG, 0x0003);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_FCR_REG, 0x0007);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_IER_REG, 0x0002);
    ithWriteRegA(ITH_UART2_BASE + ITH_UART_ACR_REG, 0x0003);
 */
    ithWriteRegA(   ITH_UART2_BASE + 0x0C,  0x0080);
    ithWriteRegA(   ITH_UART2_BASE + 0x08,  0x0001);
    ithWriteRegA(   ITH_UART2_BASE + 0x00,  0x0008);
    ithWriteRegA(   ITH_UART2_BASE + 0x04,  0x0002);
    ithWriteRegA(   ITH_UART2_BASE + 0x10,  0x000d);
    ithWriteRegA(   ITH_UART2_BASE + 0x0C,  0x0003);
    ithWriteRegA(   ITH_UART2_BASE + 0x08,  0x0007);
    ithWriteRegA(   ITH_UART2_BASE + 0x04,  0x0002);
    ithWriteRegA(   ITH_UART2_BASE + 0x24,  0x0003);
}

static void _wiegand_set_uart3(void)
{
    //uart3 Rx Source from wg1
    //ithWriteRegMaskA(ITH_GPIO_BASE+0x1A0, (1 << 31), (1 << 31));
/*
    ithWriteRegA(ITH_UART3_BASE + 0x0C, 0x0080);
    ithWriteRegA(ITH_UART3_BASE + 0x08, 0x0001);
    ithWriteRegA(ITH_UART3_BASE + 0x00, 0x0008);
    ithWriteRegA(ITH_UART3_BASE + 0x04, 0x0002);
    ithWriteRegA(ITH_UART3_BASE + 0x10, 0x000d);
    ithWriteRegA(ITH_UART3_BASE + 0x0C, 0x0003);
    ithWriteRegA(ITH_UART3_BASE + 0x08, 0x0007);
    ithWriteRegA(ITH_UART3_BASE + 0x04, 0x0002);
    ithWriteRegA(ITH_UART3_BASE + 0x24, 0x0003);
 */
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_LCR_REG,          0x0080);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_IIR_REG,          0x0001);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_THR_REG,          0x0008);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_IER_REG,          0x0002);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_IIR_TXFIFOFULL,   0x000d);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_LCR_REG,          0x0003);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_FCR_REG,          0x0007);
    ithWriteRegA(   ITH_UART3_BASE + ITH_UART_IER_REG,          0x0002);
    ithWriteRegA(   ITH_UART3_BASE + 0x24,                      0x0003);
}

void init_wiegand_controller(WIEGANDID id)
{
    uint32_t    data = 0;
    int         d0pin = -1, d1pin = -1;

    if (id)
    {
        d0pin = 86; d1pin = 87;
    }
    else
    {
        d0pin = 88; d1pin = 89;
    }

    //set wiegand0, wiegand1 out
    ithGpioSetMode( 45, ITH_GPIO_MODE2);
    ithGpioSetMode( 62, ITH_GPIO_MODE2);

    //UART Setting  , uart2 for wg0, uart3 for wg1
    _wiegand_set_uart2();
    _wiegand_set_uart3();
    _wiegand_reset(id);
    _wiegand_set_pins(id, d0pin, d1pin);
    _wiegand_controller_enable(id, 1);
    _wiegand_parity_enable(id, 1);
    _wiegand_set_prescale(id, 0x4320);
    _wiegand_set_timeout(id, 0x012C);
    _wiegand_set_uartdiv(id, 0x208d);
    _wiegand_set_debounce(id, 0x1);
    _wiegand_set_intr_clear(id, 1);
    _wiegand_set_probsel(id, 0x2);
    _wiegand_set_mode(id, 0);

    printf("--- wiegand init finished ---\n");
}

void *TestFunc(void *arg)
{
    printf("test wiegand start\n");
    init_wiegand_controller(1);
    init_wiegand_controller(0);
}