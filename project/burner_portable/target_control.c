#include <sys/ioctl.h>
#include <unistd.h>
#include "ite/mmp_types.h"

#define HOST_16bMODE 0
#define HOST_32bMODE 1

unsigned long m_GPIOSEL;
MMP_UINT16 HOST_MODE = HOST_32bMODE;
int DummyData=8;

MMP_UINT16 CHIP_ID = 0;

int WrBen;
int SPISTART;
MMP_UINT16 SPI_IDREAD = 0;

MMP_UINT8 wrBuf[128];
MMP_UINT8 rdBuf[128];



static MMP_UINT32 SpiRead(MMP_UINT32 wrLen, MMP_UINT8* pwrBuf, MMP_UINT32 rdLen, MMP_UINT8* prdBuf)
{

    return 0;
}

static MMP_ULONG SpiWrite(MMP_UINT32 ctrlLen, MMP_UINT8* pCtrlBuf, MMP_UINT32 dataLen, MMP_UINT8* pDataBuf)
{

    return 0;
}

#if 1

MMP_RESULT
HOST_WriteRegister(
    MMP_UINT32 addr,
    MMP_UINT32 value)
{

	write_slave_register(addr,value);

	return MMP_RESULT_SUCCESS;

}
	
MMP_RESULT
HOST_ReadRegister(
    MMP_UINT32 addr,
    MMP_UINT32 *value,
	MMP_UINT16 mode)
{

	*value=read_slave_register_value(addr);
	return MMP_RESULT_SUCCESS;
}


#else



//=============================================================================
/**
 * Write HOST Register
 *
 * @return 0 If successful. Otherwise, return a nonzero value.
 */
//=============================================================================
MMP_RESULT
HOST_WriteRegister(
    MMP_UINT32 addr,
    MMP_UINT32 value)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;

//	if(CHIP_ID == 0x2300 || CHIP_ID == 0x5800 || CHIP_ID == 0x6800 ||CHIP_ID == 0x9070 || CHIP_ID == 0x9910)
	if(CHIP_ID == 0x2300 || CHIP_ID == 0x5800 || CHIP_ID == 0x6800 ||CHIP_ID == 0x9070 || CHIP_ID == 0x9910 || CHIP_ID == 0x9850)
	{
		addr /= 2;
		wrLen = 3;
		rdLen = 2;

		wrBuf[0] = 0;
		wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
		wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);
		rdBuf[0] = (MMP_UINT8)(value & 0x00FF);
		rdBuf[1] = (MMP_UINT8)((value & 0xFF00) >> 8);

		result = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);
	}
	else
	{

		wrLen = 5;
		/*
		if(WrBen == 0)
		{
			rdLen = 4;
		}
		else if(WrBen == 1)
		{
			rdLen = 2;
		}
		else if(WrBen == 2)
		{
			rdLen = 2;
			addr += 2;
		}
		else if(WrBen == 3)
		{
			rdLen = 1;
		}
		else if(WrBen == 4)
		{
			rdLen = 1;
			addr += 1;
		}
		else if(WrBen == 5)
		{
			rdLen = 1;
			addr += 2;
		}
		else
		{
			rdLen = 1;
			addr += 3;
		}
		*/
		
		rdLen = 4; //always WrBen=0

		wrBuf[0] = 4;
		wrBuf[1] = (MMP_UINT8)(addr & 0x000000FF);
		wrBuf[2] = (MMP_UINT8)((addr & 0x0000FF00) >> 8);
		wrBuf[3] = (MMP_UINT8)((addr & 0x00FF0000) >> 16);
		wrBuf[4] = (MMP_UINT8)((addr & 0xFF000000) >> 24);
		rdBuf[0] = (MMP_UINT8)(value & 0x000000FF);
		rdBuf[1] = (MMP_UINT8)((value & 0x0000FF00) >> 8);
		rdBuf[2] = (MMP_UINT8)((value & 0x00FF0000) >> 16);
		rdBuf[3] = (MMP_UINT8)((value & 0xFF000000) >> 24);

		result = SpiWrite(wrLen, &wrBuf, rdLen, &rdBuf);
	}
    return result;
}

MMP_RESULT
HOST_ReadRegister(
    MMP_UINT32 addr,
    MMP_UINT32 *value,
	MMP_UINT16 mode)
{
    MMP_RESULT result = MMP_SUCCESS;
    MMP_UINT32 wrLen;
    MMP_UINT32 rdLen;
	MMP_UINT8  prereadstaus;
/*	//use another API for ID read case
	if(SPISTART | SPI_IDREAD){
		DummyData = 4;
	}
*/
	if(mode == HOST_16bMODE) //never enter this in 970 960
	{
		addr /= 2;

		wrLen = 3;
		rdLen = 2;

		wrBuf[0] = 1;
		wrBuf[1] = (MMP_UINT8)(addr & 0x00FF);
		wrBuf[2] = (MMP_UINT8)((addr & 0xFF00) >> 8);

		result = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

		*value = (MMP_UINT16)((rdBuf[1] << 8) + rdBuf[0]);
	}
	else
	{
		if(DummyData == 8){
			wrLen = 9;
			wrBuf[0] = 0x0E;
			wrBuf[1] = (MMP_UINT8)(addr & 0x000000FF);
			wrBuf[2] = (MMP_UINT8)((addr & 0x0000FF00) >> 8);
			wrBuf[3] = (MMP_UINT8)((addr & 0x00FF0000) >> 16);
			wrBuf[4] = (MMP_UINT8)((addr & 0xFF000000) >> 24);
			wrBuf[5] = 4;
			wrBuf[6] = 0;
			wrBuf[7] = 0;
			wrBuf[8] = 0;

			SpiWrite(wrLen, &wrBuf, 0, &rdBuf);

			wrBuf[0] = 0x0F;
			do{
			SpiRead(1, &wrBuf, 1, &prereadstaus);
			prereadstaus = (prereadstaus & 0x1f)*4;
			}while(prereadstaus == 0);

			wrLen = 5;
			wrBuf[0]  = 0x05;
		} else {
			wrLen = 5+DummyData;
			wrBuf[0]  = 0x05 | (DummyData << 4);
		}

		rdLen = 4;

		//wrBuf[0]  = 0x05 | (DummyData << 4);
		wrBuf[1]  = (MMP_UINT8)(addr & 0x000000FF);
		wrBuf[2]  = (MMP_UINT8)((addr & 0x0000FF00) >> 8);
		wrBuf[3]  = (MMP_UINT8)((addr & 0x00FF0000) >> 16);
		wrBuf[4]  = (MMP_UINT8)((addr & 0xFF000000) >> 24);
		wrBuf[5]  = 0;
		wrBuf[6]  = 0;
		wrBuf[7]  = 0;
		wrBuf[8]  = 0;
		wrBuf[9]  = 0;
		wrBuf[10] = 0;
		wrBuf[11] = 0;
		wrBuf[12] = 0;

		result = SpiRead(wrLen, &wrBuf, rdLen, &rdBuf);

		if(result)
		{
			*value = 0;
		}else
		{
			*value = (MMP_UINT32)((rdBuf[3] << 24) + (rdBuf[2] << 16) +(rdBuf[1] << 8) + rdBuf[0]);
		}
	}


	return result;
}
#endif
void gpio_set(unsigned int m_GPIOSEL)
{
//	void CPropertyPage6::OnBnClickedButton1()
	{
		unsigned long GPIO_DIR;
		unsigned long GPIO_OUT;
		unsigned long GPIO_IN;
		unsigned long GPIONUM_tmp;
		unsigned long GPIOMODE_tmp;
	
	
	
		GPIONUM_tmp = (m_GPIOSEL % 8)*4;
	
		if(m_GPIOSEL <= 7){
			HOST_ReadRegister(0xD1000060, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000060, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 15){
			HOST_ReadRegister(0xD1000064, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000064, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 23){
			HOST_ReadRegister(0xD1000068, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000068, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 31){
			HOST_ReadRegister(0xD100006c, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD100006c, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 39){
			HOST_ReadRegister(0xD10000E0, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD10000E0, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 47){
			HOST_ReadRegister(0xD10000E4, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD10000E4, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 55){
			HOST_ReadRegister(0xD10000E8, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD10000E8, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 63){
			HOST_ReadRegister(0xD10000EC, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD10000EC, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 71){
			HOST_ReadRegister(0xD1000160, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000160, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 79){
			HOST_ReadRegister(0xD1000164, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000164, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 87){
			HOST_ReadRegister(0xD1000168, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD1000168, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 95){
			HOST_ReadRegister(0xD100016C, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD100016C, GPIOMODE_tmp);
		}else if(m_GPIOSEL <= 103){
			HOST_ReadRegister(0xD10001E0, &GPIOMODE_tmp , HOST_MODE);
			GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
			HOST_WriteRegister(0xD10001E0, GPIOMODE_tmp);
		}
	
		if(m_GPIOSEL <= 31)
		{
			HOST_ReadRegister(0xd1000008, &GPIO_DIR , HOST_MODE);
			HOST_ReadRegister(0xd1000000, &GPIO_OUT , HOST_MODE);
			GPIO_DIR |= (0x1 << m_GPIOSEL);
			GPIO_OUT |= (0x1 << m_GPIOSEL);
			HOST_WriteRegister(0xD100000C, GPIO_OUT);
			HOST_WriteRegister(0xD1000008, GPIO_DIR);
	
			HOST_ReadRegister(0xd1000004, &GPIO_IN , HOST_MODE);
			GPIO_IN = (GPIO_IN >> m_GPIOSEL) & 0x1;
			if(GPIO_IN == 1){
					//str1.Format(_T("GPIO%d set 1 pass"),m_GPIOSEL);
					//MessageBox(str1);
					printf("GPIO%d set 1 pass \n",m_GPIOSEL);
			}else{
					printf("GPIO%d set 1 fail(read back from GPIO%d is 0)\n",m_GPIOSEL,m_GPIOSEL);
			}
		}
		else if(m_GPIOSEL <= 63)
		{
			HOST_ReadRegister(0xd1000088, &GPIO_DIR , HOST_MODE);
			HOST_ReadRegister(0xd1000080, &GPIO_OUT , HOST_MODE);
			GPIO_DIR |= (0x1 << (m_GPIOSEL-32));
			GPIO_OUT |= (0x1 << (m_GPIOSEL-32));
			HOST_WriteRegister(0xD100008C, GPIO_OUT);
			HOST_WriteRegister(0xD1000088, GPIO_DIR);
	
			HOST_ReadRegister(0xd1000084, &GPIO_IN , HOST_MODE);
			GPIO_IN = (GPIO_IN >> (m_GPIOSEL-32)) & 0x1;
			if(GPIO_IN == 1){
					printf("GPIO%d set 1 pass \n",m_GPIOSEL);

			}else{
					printf("GPIO%d set 1 fail(read back from GPIO%d is 0)\n",m_GPIOSEL,m_GPIOSEL);

			}
		}
		else if(m_GPIOSEL <= 95)
		{
			HOST_ReadRegister(0xd1000108, &GPIO_DIR , HOST_MODE);
			HOST_ReadRegister(0xd1000100, &GPIO_OUT , HOST_MODE);
			GPIO_DIR |= (0x1 << (m_GPIOSEL-64));
			GPIO_OUT |= (0x1 << (m_GPIOSEL-64));
			HOST_WriteRegister(0xD100010C, GPIO_OUT);
			HOST_WriteRegister(0xD1000108, GPIO_DIR);
	
			HOST_ReadRegister(0xd1000104, &GPIO_IN , HOST_MODE);
			GPIO_IN = (GPIO_IN >> (m_GPIOSEL-64)) & 0x1;
			if(GPIO_IN == 1){
					printf("GPIO%d set 1 pass \n",m_GPIOSEL);

			}else{
					printf("GPIO%d set 1 fail(read back from GPIO%d is 0)\n",m_GPIOSEL,m_GPIOSEL);

			}
		}
		else if(m_GPIOSEL <= 127)
		{
			HOST_ReadRegister(0xd1000188, &GPIO_DIR , HOST_MODE);
			HOST_ReadRegister(0xd1000180, &GPIO_OUT , HOST_MODE);
			GPIO_DIR |= (0x1 << (m_GPIOSEL-96));
			GPIO_OUT |= (0x1 << (m_GPIOSEL-96));
			HOST_WriteRegister(0xD100018C, GPIO_OUT);
			HOST_WriteRegister(0xD1000188, GPIO_DIR);
	
			HOST_ReadRegister(0xd1000184, &GPIO_IN , HOST_MODE);
			GPIO_IN = (GPIO_IN >> (m_GPIOSEL-96)) & 0x1;
			if(GPIO_IN == 1){
					printf("GPIO%d set 1 pass \n",m_GPIOSEL);

			}else{
					printf("GPIO%d set 1 fail(read back from GPIO%d is 0)\n",m_GPIOSEL,m_GPIOSEL);

			}
		}
	}

}

void gpio_clear(unsigned int m_GPIOSEL)
{
	unsigned long GPIO_DIR;
	unsigned long GPIO_OUT;
	unsigned long GPIO_IN;
	unsigned long GPIONUM_tmp;
	unsigned long GPIOMODE_tmp;



	GPIONUM_tmp = (m_GPIOSEL % 8)*4;

	if(m_GPIOSEL <= 7){
		HOST_ReadRegister(0xD1000060, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000060, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 15){
		HOST_ReadRegister(0xD1000064, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000064, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 23){
		HOST_ReadRegister(0xD1000068, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000068, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 31){
		HOST_ReadRegister(0xD100006c, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD100006c, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 39){
		HOST_ReadRegister(0xD10000E0, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD10000E0, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 47){
		HOST_ReadRegister(0xD10000E4, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD10000E4, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 55){
		HOST_ReadRegister(0xD10000E8, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD10000E8, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 63){
		HOST_ReadRegister(0xD10000EC, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD10000EC, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 71){
		HOST_ReadRegister(0xD1000160, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000160, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 79){
		HOST_ReadRegister(0xD1000164, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000164, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 87){
		HOST_ReadRegister(0xD1000168, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD1000168, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 95){
		HOST_ReadRegister(0xD100016C, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD100016C, GPIOMODE_tmp);
	}else if(m_GPIOSEL <= 103){
		HOST_ReadRegister(0xD10001E0, &GPIOMODE_tmp , HOST_MODE);
		GPIOMODE_tmp = ~(0xf << GPIONUM_tmp) & GPIOMODE_tmp;
		HOST_WriteRegister(0xD10001E0, GPIOMODE_tmp);
	}

	if(m_GPIOSEL <= 31)
	{
		HOST_ReadRegister(0xd1000008, &GPIO_DIR , HOST_MODE);
		HOST_ReadRegister(0xd1000000, &GPIO_OUT , HOST_MODE);
		GPIO_DIR |= (0x1 << m_GPIOSEL);
		GPIO_OUT = (0x1 << m_GPIOSEL);
		HOST_WriteRegister(0xD1000010, GPIO_OUT);
		HOST_WriteRegister(0xD1000008, GPIO_DIR);

		HOST_ReadRegister(0xd1000004, &GPIO_IN , HOST_MODE);
		GPIO_IN = (GPIO_IN >> m_GPIOSEL) & 0x1;
		if(GPIO_IN == 0){
				printf("GPIO%d set 0 pass",m_GPIOSEL);
		}else{
				printf("GPIO%d set 0 fail(read back from GPIO%d is 1)",m_GPIOSEL,m_GPIOSEL);
		}
	}
	else if(m_GPIOSEL <= 63)
	{
		HOST_ReadRegister(0xd1000088, &GPIO_DIR , HOST_MODE);
		HOST_ReadRegister(0xd1000080, &GPIO_OUT , HOST_MODE);
		GPIO_DIR |= (0x1 << (m_GPIOSEL-32));
		GPIO_OUT = (0x1 << (m_GPIOSEL-32));
		HOST_WriteRegister(0xD1000090, GPIO_OUT);
		HOST_WriteRegister(0xD1000088, GPIO_DIR);

		HOST_ReadRegister(0xd1000084, &GPIO_IN , HOST_MODE);
		GPIO_IN = (GPIO_IN >> (m_GPIOSEL-32)) & 0x1;
		if(GPIO_IN == 0){
				printf("GPIO%d set 0 pass",m_GPIOSEL);

		}else{
				printf("GPIO%d set 0 fail(read back from GPIO%d is 1)",m_GPIOSEL,m_GPIOSEL);

		}
	}
	else if(m_GPIOSEL <= 95)
	{
		HOST_ReadRegister(0xd1000108, &GPIO_DIR , HOST_MODE);
		HOST_ReadRegister(0xd1000100, &GPIO_OUT , HOST_MODE);
		GPIO_DIR |= (0x1 <<(m_GPIOSEL-64));
		GPIO_OUT = (0x1 <<(m_GPIOSEL-64));
		HOST_WriteRegister(0xD1000110, GPIO_OUT);
		HOST_WriteRegister(0xD1000108, GPIO_DIR);

		HOST_ReadRegister(0xd1000104, &GPIO_IN , HOST_MODE);
		GPIO_IN = (GPIO_IN >> (m_GPIOSEL-64)) & 0x1;
		if(GPIO_IN == 0){
				printf("GPIO%d set 0 pass",m_GPIOSEL);

		}else{
				printf("GPIO%d set 0 fail(read back from GPIO%d is 1)",m_GPIOSEL,m_GPIOSEL);

		}
	}
	else if(m_GPIOSEL <= 127)
	{
		HOST_ReadRegister(0xd1000188, &GPIO_DIR , HOST_MODE);
		HOST_ReadRegister(0xd1000180, &GPIO_OUT , HOST_MODE);
		GPIO_DIR |= (0x1 << (m_GPIOSEL-96));
		GPIO_OUT = (0x1 << (m_GPIOSEL-96));
		HOST_WriteRegister(0xD1000190, GPIO_OUT);
		HOST_WriteRegister(0xD1000188, GPIO_DIR);

		HOST_ReadRegister(0xd1000184, &GPIO_IN , HOST_MODE);
		GPIO_IN = (GPIO_IN >> (m_GPIOSEL-96)) & 0x1;
		if(GPIO_IN == 0){
				printf("GPIO%d set 0 pass",m_GPIOSEL);

		}else{
				printf("GPIO%d set 0 fail(read back from GPIO%d is 1)",m_GPIOSEL,m_GPIOSEL);

		}
	}
}

/*
glamomem -t glamomem.dat -i -q
glamomem -t glamomem.dat -l my.bin
glamomem -t glamomem.dat -R 0x00000001 -a 0xd8300000
*/


void target_script_load()
{
	char FileName[1024];
	
	HOST_WriteRegister(0xd8000004, 0x00000004);
	glamomcu_load_init_script("E:/IT9860_360Mhz_DDR2_360Mhz.txt");
	usleep(1000);
	glamomcu_load_ram("E:/burner_portable.bin");
	HOST_WriteRegister(0xd8300000, 0x00000001);

}

