#include <windows.h>
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <stdio.h>
#include <cstdlib>
#include <atlbase.h>

#include "Engine.h"
#include "Utils.h"
#include <openvr.h>

using namespace System;
using namespace System::Windows::Forms;

using namespace Parsers;

Engine::Engine()
	: HMD(NULL)
	, RenderModels(NULL)
	, CompanionWindow(NULL)
	, CompanionWindowWidth(1024)
	, CompanionWindowHeight(512)
	, Devices(NULL)
{
}

bool Engine::Init()
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		array<Object^>^ params =  { __FUNCTION__, gcnew String(SDL_GetError()) };
		dprintf("%s - SDL could not initialize! SDL Error: %s\n", params);
		return false;
	}

	vr::EVRInitError eError = vr::VRInitError_None;

	HMD = vr::VR_Init(&eError, vr::VRApplication_Scene);
	if (eError != vr::VRInitError_None)
	{
		HMD = NULL;
		array<Object^>^ params = { gcnew String(vr::VR_GetVRInitErrorAsEnglishDescription(eError)) };
		auto err = String::Format("Unable to init VR runtime: %s", params);
		MessageBox::Show(err, "VR_Init Failed", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	RenderModels = (vr::IVRRenderModels*) vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &eError);
	if (!RenderModels)
	{
		HMD = NULL;
		vr::VR_Shutdown();

		array<Object^>^ params = { gcnew String(vr::VR_GetVRInitErrorAsEnglishDescription(eError)) };
		auto err = String::Format("Unable to get render model interface: %s", params);
		MessageBox::Show(err, "VR_Init Failed", MessageBoxButtons::OK, MessageBoxIcon::Error);
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
			array<Object^>^ params = { __FUNCTION__, gcnew String(SDL_GetError()) };
			dprintf("%s - Window could not be created! SDL Error: %s\n", params);
			return false;
		}

	}

	if (!InitD3D12())
	{
		array<Object^>^ params = { __FUNCTION__ };
		dprintf("%s - Unable to initialize D3D12!\n", params);
		return false;
	}

	return true;
}

bool Engine::InitD3D12()
{
	UINT DXGIFactoryFlags = 0;

	if (CommandLine::dxDebug)
	{
		CComPtr< ID3D12Debug > DebugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
		{
			DebugController->EnableDebugLayer();
			DXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
		}
	}

	CComPtr<IDXGIFactory6> Factory;
	if (FAILED(CreateDXGIFactory2(DXGIFactoryFlags, IID_PPV_ARGS(&Factory))))
	{
		dprintf("CreateDXGIFactory2 failed.\n", gcnew array<Object^>(0));
		return false;
	}

	int primaryAdapterIndex = 0;
	HMD->GetDXGIOutputInfo(&primaryAdapterIndex);
	for (UINT adapterIndex = 0; ; adapterIndex++)
	{
		Device* fnDevice = new Device();
		ID3D12Device4* dxDevice = nullptr;
		IDXGIAdapter1* Adapter = nullptr;
		if (DXGI_ERROR_NOT_FOUND == Factory->EnumAdapters1(adapterIndex, &Adapter))
			break;
		DXGI_ADAPTER_DESC1 Desc;
		Adapter->GetDesc1(&Desc);
		String^ name = gcnew String(Desc.Description);
		if (name != "Microsoft Basic Render Driver" || adapterIndex == primaryAdapterIndex)  //Surely there's a better way to do this??
		{
			if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dxDevice))))
			{
				fnDevice->Device = dxDevice;
				fnDevice->Desc = Desc;
				if (adapterIndex == primaryAdapterIndex)
					Devices.insert(Devices.begin(), fnDevice);
				else
					Devices.push_back(fnDevice);
			}
		}
		Adapter->Release();

	}

	for (auto devItr = Devices.begin(); devItr != Devices.end(); devItr++)
	{
		auto device = *devItr;
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		if (FAILED(device->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&device->GraphicsQueue))))
		{
			array<Object^>^ params = {gcnew String(device->Desc.Description)};
			dprintf("Failed to create graphics queue on device: %s\n", params);
		}
		queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		if (FAILED(device->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&device->ComputeQueue))))
		{
			array<Object^>^ params = { gcnew String(device->Desc.Description) };
			dprintf("Failed to create compute queue on device: %s\n", params);
		}
	}

	if (Devices.empty())
	{
		MessageBox::Show("No DirectX 12 capable GPU detected!", "Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
		return false;
	}

	if (CommandLine::useCompanionWindow)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = g_nFrameCount;
		swapChainDesc.Width = CompanionWindowWidth;
		swapChainDesc.Height = CompanionWindowHeight;
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		struct SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(CompanionWindow, &wmInfo);
		HWND hWnd = wmInfo.info.win.window;

		if (FAILED(Factory->CreateSwapChainForHwnd(Devices[0]->GraphicsQueue, hWnd, &swapChainDesc, nullptr, nullptr, &SwapChain)))
		{
			dprintf("Failed to create DXGI swapchain for companion window.\n", gcnew array<Object^>(0));
			return false;
		}

		Factory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	}

	return true;
}

void Engine::Shutdown()
{
	if (HMD)
	{
		vr::VR_Shutdown();
		HMD = NULL;
	}

	for (auto devItr = Devices.begin(); devItr != Devices.end(); devItr++)
	{
		delete *devItr;
	}

	if (CompanionWindow)
	{
		SDL_DestroyWindow(CompanionWindow);
		CompanionWindow = NULL;
	}

	SDL_Quit();
}


Engine::~Engine()
{
	dprintf("Shutdown", gcnew array<Object^>(0));
}
