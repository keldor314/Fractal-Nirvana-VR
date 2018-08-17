#pragma once

#include <openvr.h>

class Engine
{
public:
	Engine();
	bool Init();
	~Engine();

private:
	vr::IVRSystem *m_pHMD;
};

