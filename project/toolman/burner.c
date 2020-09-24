#include "ite/itu.h"
#include "project.h"
#include "scene.h"

#include "ssp/mmp_spi.h"

static pthread_t burnTask;

void set_io_high(unsigned int pin)
{
	ithGpioSetOut(pin);
	ithGpioSetMode(pin, ITH_GPIO_MODE0);
	ithGpioSet(pin);

}
void set_io_low(unsigned int pin)
{
	ithGpioSetOut(pin);
	ithGpioSetMode(pin, ITH_GPIO_MODE0);
	ithGpioClear(pin);

}

void switch_to_channel(int channel)
{
switch(channel)
{
	case 1:
		set_io_low(48);
		set_io_low(49);
		set_io_low(50);
		break;
	case 2:
		set_io_low(48);
		set_io_low(49);
		set_io_high(50);

		break;
	case 3:
		set_io_low(48);
		set_io_high(49);
		set_io_low(50);
		break;
	case 4:
		set_io_low(48);
		set_io_high(49);
		set_io_high(50);
		break;
	case 5:
		
		set_io_high(48);
		set_io_high(49);
		set_io_low(50);
		break;
	default:
		printf("not defined ");
		break;
		

}

}


int get_slave_chip_id()
{
	uint32_t chip_id;

	uint8_t indata[16]={0};
	read_slave_register(0xd8000004,indata);//packet3
	for(int i=0;i<9;i++)
	printf("0x%x ",indata[i]);	
		printf("\n");

	if(indata[3]==0x9 && indata[2]==0x60)
	{
		printf("chip id is %x%x",indata[2],indata[3]);
		return 1;
	}
	else
		return 0;

}


int set_bypass_mode()
{
	printf("set_bypass_mode \n");

	write_slave_register(0xd1000230,0x800a8005);	
	write_slave_register(0xd1000234,0x80078006);	
	write_slave_register(0xd1000204,0x02000000);	

	return 0;
}

void write_slave_register(uint32_t addr,uint32_t data)
{
	uint8_t cmd[9] ={0x0};
	uint8_t indata[9]={0};
	uint32_t *tmp_u32=0;
	uint32_t addr_endian;
	uint32_t data_endian;

	cmd[0]=0x04;
	//addr_endian= BLEndianUint32(addr);
	addr_endian= addr;
	tmp_u32=(uint32_t*)&cmd[1];
	memcpy(tmp_u32,&addr_endian,4);

	//data_endian= BLEndianUint32(data);
	data_endian= data; 

	tmp_u32=(uint32_t*)&cmd[5];
	
	memcpy(tmp_u32,&data_endian,4);


	mmpSpiPioWrite(SPI_1, cmd , 9, 0,0, 0);


}


void read_slave_register(uint32_t addr,uint8_t* indata )
{
    uint8_t cmd[9] ={0x0};

	uint32_t *cmd_reg_addr=0;
	uint32_t addr_endian;

	cmd[0]=0x45;
	//addr_endian= BLEndianUint32(addr);
	addr_endian= addr;
	cmd_reg_addr=(uint32_t*)&cmd[1];
	memcpy(cmd_reg_addr,&addr_endian,4);

	mmpSpiPioRead(SPI_1, cmd , 9, indata,4, 4);
/*
	printf("read_slave_register addr 0x%x\n",addr);
	for(int i=0;i<9;i++)
	printf("0x%x ",indata[i]);	
		printf("\n");
*/
	
}



void writing_progressing(uint32_t i)
{
	
#define ROM_SIZE_WRITE_PIECE  1024*1024


	uint8_t* rom_content=NULL;
	uint8_t* rom_read_content=NULL;
	rom_content = malloc(ROM_SIZE_WRITE_PIECE);
	rom_read_content = malloc(ROM_SIZE_WRITE_PIECE);

	if( (NULL==rom_content)||(NULL==rom_read_content) )
	{
		printf("rom_content ERROR \n");
		return;
	}

	char filepath_tmp[64];
	uint32_t read_size;
	FILE* fp;
	uint32_t addr;

	//for(i=1;i<=5;i++)
	{
			snprintf(filepath_tmp,64,"E:/%d.rom",i);
			fp = fopen(filepath_tmp, "r");
			if (NULL == fp)
				printf("fp can not open %s\n",filepath_tmp);


			Nor2nd_Init();


			uint32_t tick;
			//tick=SDL_GetTicks();
			printf("Nor2nd_Write start \n");
			tick = xTaskGetTickCount();


			addr=0x0;//write from 0 address
			while(1)
			{
				read_size=fread(rom_content,1,ROM_SIZE_WRITE_PIECE,fp);
				if(0==read_size)
					break;

				printf("Nor2nd_Write addr=0x%x,size=0x%x \n",addr,read_size);
				Nor2nd_Write(addr,rom_content,read_size);
				printf("Nor2nd_Read addr=0x%x \n",addr);
				
				Nor2nd_Read(addr,rom_read_content,read_size);
				printf("Nor2nd_Compare addr=0x%x \n",addr);
				if(0 != memcmp(rom_content,rom_read_content,read_size ) )
				{
					printf("WRITE ERROR in address 0x%x \n",addr);
					break;
				}
				addr+=read_size;

			}	

		printf("Nor2nd_Write finished elapsed %d \n",elased_tick(tick));

	}

}


static void* burnningTask(void* arg)
{
	uint32_t i;
	int ret;
	mmpSpiInitialize(SPI_1, SPI_OP_MASTR, CPO_0_CPH_0, SPI_CLK_5M);

	
	sleep(1);
	while(1)
	{
		//for(i=1;i<=5;i++)
		i=1;
		{
			printf("channel=%d\n",i);
			switch_to_channel(i);
			// get id 
			ret=get_slave_chip_id();
			if(ret)
			{
				//chip is ready to existed

				//bypass mode
				set_bypass_mode();
				writing_progressing(i);
				//burn_slave_nor();
			}
			sleep(1);

			
		}

	}
	
	
}

void burn_writer_stop()
{
   pthread_join(burnTask, NULL);
}

void burn_writer_start()
{
	printf("burn_writer_start\n");
	    pthread_create(&burnTask, NULL, burnningTask, NULL);
}
