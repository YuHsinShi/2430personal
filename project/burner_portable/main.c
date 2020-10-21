#include <sys/ioctl.h>
#include <unistd.h>

int is_in_acm_mode()
{
	return 1;
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
	cliInit();
}


while(1)
{
sleep(1);
}
burn_led_congtrol();

burn_process_start();

}

