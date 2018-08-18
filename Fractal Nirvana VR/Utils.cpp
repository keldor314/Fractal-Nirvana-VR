#include "d3dx12.h"
#include "Utils.h"

using namespace System;
using namespace System::Runtime::InteropServices;

void dprintf(String ^fmt, array<Object^>^ args)
{
	auto str = String::Format(fmt, args);
	OutputDebugStringA(stringToCharP(str));
}


char* stringToCharP(String^ str)
{
	IntPtr tmp = Marshal::StringToHGlobalAnsi(str);
	return reinterpret_cast<char*>(tmp.ToPointer());
}