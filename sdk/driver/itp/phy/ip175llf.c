#include <unistd.h>
#include "ite/ite_mac.h"


static uint16_t mdio_read(uint16_t addr, uint16_t regnum)
{
    struct mii_ioctl_data phy_reg = { 0 };
    phy_reg.phy_id = addr;
    phy_reg.reg_num = regnum;
    iteMacIoctl(&phy_reg, IOCGMIIREG);
    return phy_reg.val_read;
}

static void mdio_write(uint16_t addr, uint16_t regnum, uint16_t val)
{
    struct mii_ioctl_data phy_reg = { 0 };
    phy_reg.phy_id = addr;
    phy_reg.reg_num = regnum;
    phy_reg.val_write = val;
    iteMacIoctl(&phy_reg, IOCSMIIREG);
    return;
}

void
PhyInit(int ethMode)
{
    uint32_t timeout;
    uint16_t chip_id, val;
    int i, err;

    chip_id = mdio_read(20, 0);
    if (chip_id == 0x175D)
        printf("IP175LLF: id 0x%X == 0x175D \n", chip_id);
    else {
        printf("IP175LLF: id 0x%X != 0x175D ==> error stop!! \n", chip_id);
        while (1);
    }
	
    /* reset switch */
    mdio_write(20, 2, 0x175D);
    val = mdio_read(20, 2);
    usleep(2 * 1000);
    for (timeout = 10; timeout == 0; timeout--) {
        if (mdio_read(20, 2) == 0)
            break;
        else
            usleep(1000);
    }
	if (!timeout)
		printf("IP175LLF: reset timeout! \n");

    /* MAC5 force 100/full */
    mdio_write(20, 4, 0xA000);

    /* reset PHY 0~3 */
    for (i = 0; i < 5; i++)
        mdio_write(i, 0, 0xB100);

    for (i = 0; i < 5; i++)
        mdio_read(i, 0);

    usleep(2 * 1000);
    
    /* full reset ready. */
}

static int ip175llf_read_mode(int* speed, int* duplex)
{
    uint16_t val;

    val = mdio_read(21, 0);

    if (val & 0x40)
        (*speed) = SPEED_10;
    else
        (*speed) = SPEED_100;

    if (val & 0x80)
        (*duplex) = DUPLEX_FULL;
    else
        (*duplex) = DUPLEX_HALF;

    val = mdio_read(21, 1);

    if (val & 0x2000)
        return 0; // 0 means link up
    else
        return -1;
}

static uint32_t ip175llf_link_status = 0;  /* D[0]:link, D[28]:port 0, D[29]:port 1, D[30]:port 2, D[31]:port 3 */

static uint32_t ip175llf_get_link_status(void)
{
    uint32_t link_status = 0;
    uint16_t val;
    int i;

    for (i = 0; i <= 3; i++) {
        val = mdio_read(i, 1);
        val = mdio_read(i, 1);
        if (val & 0x4)
            link_status |= ((0x1 << (28 + i)) | 0x1);
    }
    if (ip175llf_link_status != link_status)
        printf("link: 0x%04X \n", link_status);

    ip175llf_link_status = link_status;

    return ip175llf_link_status;
}

/**
* Check interrupt status for link change. 
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = NULL;
/**
* Replace mac driver's ISR for phy's interrupt. 
*/
ITHGpioIntrHandler itpPhylinkIsr = NULL;
/**
* Returns 0 if the device reports link status up/ok 
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = ip175llf_read_mode;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = ip175llf_get_link_status;


