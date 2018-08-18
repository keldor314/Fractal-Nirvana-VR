#include <windows.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdio.h>
#include <cstdlib>

#include "Engine.h"
#include "Utils.h"
#include <openvr.h>

using namespace System;
using namespace Parsers;

Engine::Engine()
	: HMD(NULL)
	, RenderModels(NULL)
	, CompanionWindow(NULL)
	, CompanionWindowWidth(1024)
	, CompanionWindowHeight(512)
{
}

bool Engine::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		array<Object^>^ params =  { __FUNCTION__, gcnew System::String(SDL_GetError()) };
		dprintf("%s - SDL could not initialize! SDL Error: %s\n", params);
		return false;
	}

	vr::EVRInitError eError = vr::VRInitError_None;

	HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None)
	{
		HMD = NULL;
		array<Object^>^ params = { gcnew System::String(vr::VR_GetVRInitErrorAsEnglishDescription(eError)) };
		auto err = String::Format("Unable to init VR runtime: %s", params);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", stringToCharP(err), NULL);
		return false;
	}

	RenderModels = (vr::IVRRenderModels*) vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!RenderModels)
	{
		HMD = NULL;
		vr::VR_Shutdown();

		array<Object^>^ params = { gcnew System::String(vr::VR_GetVRInitErrorAsEnglishDescription(eError)) };
		auto err = String::Format("Unable to get render model interface: %s", params);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VR_Init Failed", stringToCharP(err), NULL);
		return false;
	}

	if (CommandLine::useCompanionWindow)
	{
		int WindowPosX = 700;
		int WindowPosY = 100;
		auto WindowFlags = SDL_WINDOW_SHOWN;
		CompanionWindow = SDL_CreateWindow("Fractal Nirvana VR", WindowPosX, WindowPosY, CompanionWindowWidth, CompanionWindowHeight, WindowFlags);
		if (!CompanionWindow)
		{
			array<Object^>^ params = { __FUNCTION__, gcnew System::String(SDL_GetError()) };
			dprintf("%s - Window could not be created! SDL Error: %s\n", params);
			return false;
		}
	}

	return true;
}


Engine::~Engine()
{
	dprintf("Shutdown", nullptr);
}
