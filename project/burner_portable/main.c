#include <sys/ioctl.h>
#include <unistd.h>

int is_in_acm_mode()
{
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
	cliInit();
}



burn_led_congtrol();

burn_process_start();

}

