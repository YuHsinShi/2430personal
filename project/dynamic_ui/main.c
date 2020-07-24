#include <unistd.h>
#include "SDL/SDL.h"
#include "ite/itp.h"
#include "audio_mgr.h"
#include "dynamic_ui.h"

int SDL_main(int argc, char *argv[])
{
    int ret = 0;

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());

    ConfigLoad();
	smtkAudioMgrInitialize();
    SceneInit();
    SceneLoad();
    ret = SceneRun();

    SceneExit();
	smtkAudioMgrTerminate();

    exit(ret);
    return ret;
}
