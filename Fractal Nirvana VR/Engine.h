#pragma once

#include <openvr.h>
#include <SDL_syswm.h>
#include <atlbase.h>
#include <d3d12.h>
#include <vector>

class Engine
{
public:
	Engine();
	bool Init();
	bool InitD3D12();
	~Engine();

private:
	vr::IVRSystem *HMD;
	vr::IVRRenderModels *RenderModels;
	SDL_Window *CompanionWindow;
	int CompanionWindowWidth;
	int CompanionWindowHeight;

	//The device the headset is plugged into will always be the first element in the vector
	std::vector<CComPtr<ID3D12Device4>> Devices; 
	
};

