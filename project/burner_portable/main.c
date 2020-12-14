#include <sys/ioctl.h>
#include <unistd.h>



#define USB_MODE_MSC 0
#define USB_MODE_ACM 1
static int mode=USB_MODE_MSC;


void set_mode_acm()
{
	printf("###SET ACM MODE ###\n");

	mode = USB_MODE_ACM;
}

int is_in_acm_mode()
{


	if(USB_MODE_ACM == mode)
		return 1;
	else	
		return 0;
}

int is_in_msc_mode()
{
	if(	is_in_acm_mode())
		return 0;
	else
		return 1;
}


int SDL_main(int argc, char *argv[])
{
if(	is_in_acm_mode())
{
	printf("###ENTER ACM MODE ###\n");

	cliInit();
}
else
{
	printf("###ENTER MSC MODE ###\n");
}


//law_test();

burn_led_congtrol();
int burn_task;
	while(1)
	{
		if(1== get_key_pressed())
		{
			burn_task=burn_process_start();
			printf("burn_task =%d \n",burn_task);
			pthread_join(burn_task,NULL);
			printf("burn_task END ===\n");
		}
		usleep(50*1000);
	}
}

