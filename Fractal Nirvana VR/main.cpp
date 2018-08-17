#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>


int main(int argc, char *argv[])
{
	Engine *engine = new Engine();
	engine->Init();
	return 0;
}