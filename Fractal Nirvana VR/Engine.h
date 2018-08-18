#pragma once

#include <openvr.h>
#include <SDL_syswm.h>

class Engine
{
public:
	Engine();
	bool Init();
	~Engine();

private:
	vr::IVRSystem *HMD;
	vr::IVRRenderModels *RenderModels;
	SDL_Window *CompanionWindow;
	int CompanionWindowWidth;
	int CompanionWindowHeight;
};

