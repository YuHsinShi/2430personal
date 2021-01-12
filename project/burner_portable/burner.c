#include "ite/itu.h"
#include "ite/itp.h"
#include "project.h"
//#include "scene.h"

#include "ssp/mmp_spi.h"

#ifndef WIN32
static pthread_t burnTask;
static float percent_rom;


#define BURNER_STATUS_IDLE	0
#define BURNER_STATUS_ROM_NOT_EXISTED	-1
#define BURNER_STATUS_ROM_NOT_EXISTED	-1


#define SPI_BURNNIGN_PORT	 SPI_0



#define UPDATE_FILE_NOR_ROM_NAME  "NOR.ROM"
#define UPDATE_FILE_NOR_PKG_NAME  "NOR.PKG"
#define UPDATE_FILE_NAND_PKG_NAME  "NAND.PKG"
//#define UPDATE_FILE_NAND_PKG_NAME  "RES_32M.PKG"

static volatile bool bunrQuit;

static int flagmode;

static unsigned char* display_str[]
={"DETECTING..","NAND","NOR"};

static unsigned char* display_info_str[]
={" "," "," "};
	
char* get_burn_status_string()
{
	int ret;
		if( 1 ==flagmode) //NAND
		{

			return display_str[1];
		}
		else if( 2 ==flagmode) //NOR
		{

			return display_str[2];	
		}
		else
		{
			return display_str[0];

		}


}

char* get_burn_info_string()
{
	int ret;
		if( 1 ==flagmode) //NAND
		{
			return get_nand_name();	

		}
		else if( 2 ==flagmode) //NOR
		{
			return get_nor2nd_name();	
		}
		else
		{
			return display_info_str[0];

		}


}

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
static char rom_file_name[32]={UPDATE_FILE_NOR_ROM_NAME};

#define ROM_FILE_NAME_CH1 "1.ROM"
#define ROM_FILE_NAME_CH2 "2.ROM"
#define ROM_FILE_NAME_CH3 "3.ROM"
#define ROM_FILE_NAME_CH4 "4.ROM"
#define ROM_FILE_NAME_CH5 "5.ROM"




void switch_to_channel(int channel)
{

	switch(channel)
	{
		case 1:
			set_io_low(48);
			set_io_low(49);
			set_io_low(50);
			strcpy(rom_file_name,ROM_FILE_NAME_CH1);
			break;
		case 2:
			set_io_low(48);
			set_io_low(49);
			set_io_high(50);
			strcpy(rom_file_name,ROM_FILE_NAME_CH2);

			break;
		case 3:
			set_io_low(48);
			set_io_high(49);
			set_io_low(50);
			
			strcpy(rom_file_name,ROM_FILE_NAME_CH3);
			break;
		case 4:
			set_io_low(48);
			set_io_high(49);
			set_io_high(50);
			
			strcpy(rom_file_name,ROM_FILE_NAME_CH4);
			break;
		case 5:
			
			set_io_high(48);
			set_io_low(49);
			set_io_low(50);
			
			strcpy(rom_file_name,ROM_FILE_NAME_CH5);
			break;
		default:
			printf("not defined ");
			break;
			

	}

}



void write_slave_register(uint32_t addr, uint32_t data)
{
	printf("write_slave_register addr 0x%x value=0x%x\n",addr,data);

	uint8_t cmd[9] = { 0x0 };
	uint32_t *tmp_u32 = 0;
	uint32_t addr_endian;
	uint32_t data_endian;

	cmd[0] = 0x04;
	//addr_endian= BLEndianUint32(addr);
	addr_endian = addr;
	tmp_u32 = (uint32_t*)&cmd[1];
	memcpy(tmp_u32, &addr_endian, 4);

	//data_endian= BLEndianUint32(data);
	data_endian = data;

	tmp_u32 = (uint32_t*)&cmd[5];

	memcpy(tmp_u32, &data_endian, 4);


	mmpSpiPioWrite(SPI_BURNNIGN_PORT, cmd, 9, 0, 0, 0);


}



uint32_t read_slave_register(uint32_t addr, uint8_t* indata)
{
	uint8_t cmd[9] = { 0x0 };

	uint32_t *cmd_reg_addr = 0;
	uint32_t addr_endian;
	uint32_t value;

	cmd[0] = 0x45;
	//addr_endian= BLEndianUint32(addr);
	addr_endian = addr;
	cmd_reg_addr = (uint32_t*)&cmd[1];
	memcpy(cmd_reg_addr, &addr_endian, 4);

	mmpSpiPioRead(SPI_BURNNIGN_PORT, cmd, 9, indata, 4, 4);
	
	memcpy(&value,indata,4);
printf("read_slave_register addr 0x%x value=0x%x\n",addr,value);

	for(int i=0;i<9;i++)
	printf("0x%x ",indata[i]);
	printf("\n");
	

return value;
}
uint32_t read_slave_register_value(uint32_t addr)
{
	uint8_t indata[16];

	uint8_t cmd[9] = { 0x0 };

	uint32_t *cmd_reg_addr = 0;
	uint32_t addr_endian;
	uint32_t value;

	cmd[0] = 0x45;
	//addr_endian= BLEndianUint32(addr);
	addr_endian = addr;
	cmd_reg_addr = (uint32_t*)&cmd[1];
	memcpy(cmd_reg_addr, &addr_endian, 4);

	mmpSpiPioRead(SPI_BURNNIGN_PORT, cmd, 9, indata, 4, 4);
	
	memcpy(&value,indata,4);
/*
printf("read_slave_register addr 0x%x value=0x%x\n",addr,value);

	for(int i=0;i<9;i++)
	printf("0x%x ",indata[i]);
	printf("\n");
	*/

return value;
}

//bit value-
void write_slave_register_by_mask(uint32_t addr, uint32_t mask, uint8_t isSet)
{
	uint8_t buf[16] = { 0x0 };
	uint32_t value;

	value=read_slave_register(addr,buf);
	printf("old value = 0x%x ,mask=0x%x ",value,mask);
	if(isSet)
		value|=mask;
	else
		value&=~mask;
	
	printf("new value = 0x%x ",value);
	write_slave_register(addr,value);
	
	value=read_slave_register(addr,buf);
	printf("read back value = 0x%x \n",value);



}

int get_ite_chip_id()
{
	uint32_t chip_id;

	uint8_t indata[16]={0};
	read_slave_register(0xd8000004,indata);//packet3
	for(int i=0;i<9;i++)
	//printf("0x%x ",indata[i]);	
	//	printf("\n");

	if( (indata[3]==0x9) && (indata[2]==0x60) )
	{
		printf("chip id is %x%x \n",indata[2],indata[3]);
		return 1;
	}
	else if ( (indata[3]==0x9 && indata[2]==0x70))
		return 2;
	else
		return -1;

}

int set_bypass_mode_970()
{
	printf("set_bypass_mode \n");

	write_slave_register(0xd1000230,0x80058006);	
	write_slave_register(0xd1000234,0x80088007);	
	write_slave_register(0xd1000204,0x02000000);	

	return 0;
}

int set_bypass_mode_960()
{
	printf("set_bypass_mode \n");

	write_slave_register(0xd1000230,0x800a8005);	
	write_slave_register(0xd1000234,0x80078006);	
	write_slave_register(0xd1000204,0x02000000);	

	return 0;
}









//ret -1: upgrade fail
//ret 1: upgrade OK

int writing_nor_progressing(FILE* fp)
{
	if(NULL== fp )
		return;
#define ROM_SIZE_WRITE_PIECE  512*1024 //1024*1024


	uint8_t* rom_content=NULL;
	uint8_t* rom_read_content=NULL;


	uint32_t read_size;
	uint32_t rom_size_total;
	uint32_t addr;


			fseek(fp, 0, SEEK_END);
			rom_size_total = ftell(fp);
			fseek(fp, 0, SEEK_SET);		

			percent_rom=0.01;
			
			uint32_t tick;
			tick=SDL_GetTicks();
			printf("Nor2nd_Write start \n");
			tick = xTaskGetTickCount();

			rom_content = malloc(ROM_SIZE_WRITE_PIECE);
			rom_read_content = malloc(ROM_SIZE_WRITE_PIECE);
			
			if( (NULL==rom_content)||(NULL==rom_read_content) )
			{
				printf("rom_content ERROR \n");
				return -1;
			}

			addr=0x0;//write from 0 address
			while(1)
			{
				read_size=fread(rom_content,1,ROM_SIZE_WRITE_PIECE,fp);
				if(0==read_size)
					break;

				while(1)
				{
				
					Nor2ndErase(addr,ROM_SIZE_WRITE_PIECE);
					printf("Nor2nd_Write addr=0x%x,size=0x%x \n",addr,read_size);
					Nor2nd_Write(addr,rom_content,read_size);
					printf("Nor2nd_Read addr=0x%x \n",addr);
					
					Nor2nd_Read(addr,rom_read_content,read_size);
					printf("Nor2nd_Compare addr=0x%x \n",addr);
					if(0 != memcmp(rom_content,rom_read_content,read_size ) )
					{
						printf("WRITE ERROR in address 0x%x \n",addr);
						printf("rom_content 0x%x,0x%x,0x%x,0x%x \n",rom_content[0],rom_content[1],rom_content[2],rom_content[3]);
						printf("rom_read_content 0x%x,0x%x,0x%x,0x%x \n",rom_read_content[0],rom_read_content[1],rom_read_content[2],rom_read_content[3]);
						return -1;
					}
					
					else
					{
						break;
					}
					
				}
				addr+=read_size;
				percent_rom= (float)((float)addr/(float)rom_size_total);
					printf("progress %f percent \n",percent_rom*100);
					
				//	printf("flashing %d \n",flashing);
				//	led_flash_set_level(flashing);
				//	flashing=flashing-10;
					//BurnerOnTimer_ui_set(0,(int)(percent*100)); //update current bar
					//BurnerOnTimer_ui_set(i,(int)(percent*100));  //update rest bar

			}	

		printf("Nor2nd_Write finished elapsed %d \n",elased_tick(tick));

		free(rom_content);
		free(rom_read_content);
		return 1;

}


#define MAX_SPI_CLK 		SPI_CLK_5M
#define MIN_SPI_CLK 		SPI_CLK_5M

void target_io_write(unsigned int pin,int sethigh)
{



//DATASET_REG_ADDR[group], 0x1 << (pin & 0x1F)
unsigned int value;
unsigned int addr;// DATASET_REG_ADDR[group]
unsigned int mask;


//ithGpioSetOut(pin); //ithSetRegBitA(PINDIR_REG_ADDR[group], pin & 0x1F);

addr = ithGpioGet_GpioPinDirAddress(pin);
mask = pin & 0x1F;
write_slave_register_by_mask(addr,mask,1);


if(sethigh)
	addr = ithGpioGet_GpioDataSetAddress(pin);
else
	addr = ithGpioGet_GpioDataClearAddress(pin);

mask = 0x1 << (pin & 0x1F);
write_slave_register_by_mask(addr,mask,1);

//ithGpioSetMode(pin, ITH_GPIO_MODE0);


//ithGpioClear(pin);
//ithGpioSet(pin);



}

void target_io_read(unsigned int pin)
{

	uint8_t indata[16]={0};
	uint32_t group;
	uint32_t value ;

	group = ithGpioGet_GpioInputAddress(pin);//

	value = read_slave_register(group,indata);//packet3
	if( value & (0x1 << (pin & 0x1F)) ){	
		printf("tartget GPIO %d is HIGH \n",pin );}
	else{
		printf("tartget GPIO %d is LOW \n",pin );}
	

}

void law_test()
{
uint8_t indata[16]={0};
uint32_t group ;
printf("law_test \n");
int flag=0;
#define PIN_GPIO 		13
//	ithGpioGet_GpioRegAddressPrint(PIN_GPIO);

	SPI_CLK_LAB my_clk;

//	ithGpioSetMode(PIN_GPIO, ITH_GPIO_MODE0);
//	ithGpioSetOut(PIN_GPIO);
//	ithGpioClear(PIN_GPIO);

	for(my_clk=MAX_SPI_CLK;my_clk>=MIN_SPI_CLK;my_clk-- )
	{
		mmpSpiInitialize(SPI_BURNNIGN_PORT, SPI_OP_MASTR, CPO_0_CPH_0, my_clk);
		


		if(1==	get_ite_chip_id() )
		{
				 
#if 2
			target_script_load();
#else
			while(1)
			{
				//target_io_read(PIN_GPIO);
				//get_ite_chip_id();
				
				if(0==flag){
				//	target_io_write(PIN_GPIO,1);
					gpio_set(PIN_GPIO);
					flag =1;}
				else{
					//target_io_write(PIN_GPIO,0);	
					gpio_clear(PIN_GPIO);
					flag =0;}
				
				
				usleep(5*1000*1000);
			}

#endif
			mmpSpiTerminate(SPI_BURNNIGN_PORT);
			return;
		
		}
		mmpSpiTerminate(SPI_BURNNIGN_PORT);

	}

}


//ret 1: nor init OK
//ret -1: can not found slave device or not supported nor
int burner_auto_set_bypass()
{

	//use fastest mode to check slave
		SPI_CLK_LAB my_clk;
	int ret;

	char id[8];

	for(my_clk=MAX_SPI_CLK;my_clk>=MIN_SPI_CLK;my_clk-- )
	{
		mmpSpiInitialize(SPI_BURNNIGN_PORT, SPI_OP_MASTR, CPO_0_CPH_0, my_clk);
		ret =	get_ite_chip_id();
		//check if slave is iTE chip // change to nor mode if in iTE mode
		if(ret >0)
		{	
			if(1== ret) //960 series
			{
				set_bypass_mode_960();
				ret= 0x960;//
			}
			else if(2== ret) //970 series
			{
				set_bypass_mode_970();
				ret= 0x970;//
			}
			else
			{
				printf("unknown chip id\n");
			}
			
			mmpSpiTerminate(SPI_BURNNIGN_PORT);
			return ret;
		}
		else
		{
			mmpSpiTerminate(SPI_BURNNIGN_PORT);
		}

	}

return ret;
}

int burner_check_storage_type_nor()
{

	int ret=-1;


		SPI_CLK_LAB my_clk;

	for(my_clk=MAX_SPI_CLK;my_clk>=MIN_SPI_CLK;my_clk-- )
	{
		mmpSpiInitialize(SPI_BURNNIGN_PORT, SPI_OP_MASTR, CPO_0_CPH_0, my_clk);

		if(1==	Nor2nd_Init(SPI_BURNNIGN_PORT) ) //960 series
		{	
			ret=nor_flash_update_process();
			if(ret>0)
				return ret;
		}
		mmpSpiTerminate(SPI_BURNNIGN_PORT);

	}
	return ret;

}
int burner_check_storage_type_nand()
{

	int ret=-1;


		SPI_CLK_LAB my_clk;

	for(my_clk=MAX_SPI_CLK;my_clk>=MIN_SPI_CLK;my_clk-- )
	{
		mmpSpiInitialize(SPI_BURNNIGN_PORT, SPI_OP_MASTR, CPO_0_CPH_0, my_clk);
//sleep(1);
		if(0==check_spi_nand_id())
		{
			//sleep(1);

			ret =nand_flash_update_process();
			
			if(ret>0)
				return ret;
		}
		//sleep(1);

		mmpSpiTerminate(SPI_BURNNIGN_PORT);

	}
	return ret;

}

int burner_check_storage_type()
{

}

//check it is ROM or PKG in USB or SD card
int burner_check_update()
{

//	UpgradeSPI_NAND("NAND.PKG");
//	UpgradeSPI_NAND("NOR.PKG");


}
static void* burnningTask(void* arg)
{


	
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


static pthread_t switchTask;
static bool switchTaskQuit;


void burn_switching_task(void* arg)
{
		uint32_t i;
		int ret;
#ifndef WIN32

		
		while(!switchTaskQuit)
		{
			for(i=1;i<=5;i++)
			{
	
				switch_to_channel(i);

				BurnerOnTimer_ui_set(i); //update current bar

				burn_process();

				sleep(2);
			}
	
		}
#endif
}



void burn_switching_start()
{
	switchTaskQuit=false;


	printf("burn_writer_start\n");
	pthread_create(&switchTask, NULL, burn_switching_task, NULL);
}
void burn_switching_stop()
{

	printf("burn_writer_start\n");
	
	switchTaskQuit=true;
	pthread_join(switchTask, NULL);
}



int get_progress_percent()
{
//PKG
	//ugGetProrgessPercentage();
int ret;
	if( 1 ==flagmode)
	{
		ret = ugGetProrgessPercentage();
	}
	else if( 2 ==flagmode)
	{
		ret=  (int)(percent_rom*100);

	}
	else
	{
		ret= 0;
	}

			return ret;
}


//ret 0: can not find PKG
//ret -1: upgrade fail
//ret 1: upgrade OK

int nor_flash_update_process()
{
	FILE* fp;
	int ret;

	if( CheckUpdateFileExisted(rom_file_name)>0 )
	{
			flagmode=2;

			fp =UpdateFileOpen(rom_file_name);
			ret =writing_nor_progressing(fp);
			fclose(fp);
			return ret;
	}
	else
	{
			return 0;
	}


}
//ret 0: can not find PKG
//ret -1: upgrade fail
//ret 1: upgrade OK

int nand_flash_update_process()
{


	if(CheckUpdateFileExisted(UPDATE_FILE_NAND_PKG_NAME) > 0)
	{
		flagmode=1;
		init_nand_workaround();
		if(0 == burner_UpgradePackage(UPDATE_FILE_NAND_PKG_NAME) )
			return 1;
		else 
			return -1;


	}
	else
	{
			return 0;
	}

}



void burn_process(void* arg)
{
	int ret ;
	printf("burn_process\n");

	percent_rom=0;
	ugSetProrgessPercentage(0);
	flagmode=0;

	
	burner_auto_set_bypass();//


	if(burner_check_storage_type_nand() >= 0) //first NAND then NOR
	{
		printf("burn_process NAND END\n");
		return;
	}

	if(burner_check_storage_type_nor() >= 0)
	{
		printf("burn_process NOR END\n");
		return;

	}

}

int burn_process_start()
{

	static pthread_t burn_task;

	printf("burn_process_start\n");
	pthread_create(&burn_task, NULL, burn_process, NULL);

return burn_task;
}

void burn_led_congtrol_process(void* arg)
{
	static unsigned int counter=0;
	static int flag=0;
	int percent;
#define LED1_IO	24
#define LED2_IO 62

ithGpioSetOut(LED1_IO);
ithGpioSetMode(LED1_IO, ITH_GPIO_MODE0);
ithGpioSet(LED1_IO);
ithPrintf("burn_led_congtrol_process");
int sleep=0;
while(1)
{
		percent = get_progress_percent();

		sleep=120-percent;
		
		//ithPrintf("percent=%d",percent);
		if(0==percent)
		{
			ithGpioSetOut(LED2_IO);
			ithGpioSetMode(LED2_IO, ITH_GPIO_MODE0);
			ithGpioSet(LED2_IO);


		}
		else if(100 == percent)
		{
			ithGpioSetOut(LED2_IO);
			ithGpioSetMode(LED2_IO, ITH_GPIO_MODE0);
			ithGpioClear(LED2_IO);

		}
		else
		{

			if(flag)
			{
	
				ithGpioSetOut(LED2_IO);
				ithGpioSetMode(LED2_IO, ITH_GPIO_MODE0);
				ithGpioSet(LED2_IO);
				flag=0;
	
			}	
			else
			{
				ithGpioSetOut(LED2_IO);
				ithGpioSetMode(LED2_IO, ITH_GPIO_MODE0);
				ithGpioClear(LED2_IO);
	
				flag=1;
	
			}

		}


	

			usleep(sleep*1000);
	
}	
	

	
	
}

int get_key_pressed()
{
	static int lastPinStatus = 0;

	#define KEY_GPIO 13
	ithGpioSetMode(KEY_GPIO, ITH_GPIO_MODE0);
	ithGpioSetIn(KEY_GPIO);
	ithGpioEnable(KEY_GPIO);



	if(ithGpioGet(KEY_GPIO) != lastPinStatus)
	{
		// GPIO status has changed
		lastPinStatus = ithGpioGet(KEY_GPIO);
		if(lastPinStatus)
		{
			printf("get_key_pressed is HIGH\n");
		}
		else
		{
			printf("get_key_pressede is LOW\n");
			
			return 1;
		}
	}



	return 0;




}

int get_mode()
{

	#define KEY_GPIO 23
	ithGpioSetMode(KEY_GPIO, ITH_GPIO_MODE0);
	ithGpioSetIn(KEY_GPIO);
	ithGpioEnable(KEY_GPIO);



	if(0 == ithGpioGet(KEY_GPIO) )
	{			
		return 1; //power from 3V3 ->ACM mode
	}
	else
	{
		return 0; //power from 5V and jump is on ->MSC mode
	}



}

void burn_led_congtrol()
{

	static pthread_t burn_task;

	printf("burn_led_congtrol\n");
	pthread_create(&burn_task, NULL, burn_led_congtrol_process, NULL);

}

#endif
