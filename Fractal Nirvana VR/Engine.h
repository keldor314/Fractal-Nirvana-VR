#pragma once

#include <openvr.h>
#include <SDL_syswm.h>
#include <atlbase.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>

struct Device
{
	DXGI_ADAPTER_DESC1 Desc;
	CComPtr<ID3D12Device4> Device;
	CComPtr<ID3D12CommandQueue> GraphicsQueue;
	CComPtr<ID3D12CommandQueue> ComputeQueue;
};

class Engine
{
public:
	Engine();
	bool Init();
	bool InitD3D12();
	void Shutdown();
	~Engine();

private:
	vr::IVRSystem *HMD;
	vr::IVRRenderModels *RenderModels;
	SDL_Window *CompanionWindow;
	int CompanionWindowWidth;
	int CompanionWindowHeight;

	//The device the headset is plugged into will always be the first element in the vector
	std::vector<Device*> Devices; 
	CComPtr<IDXGISwapChain1> SwapChain;
};

static const int g_nFrameCount = 2;