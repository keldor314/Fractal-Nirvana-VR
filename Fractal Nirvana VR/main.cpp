#include "Engine.h"

#include <SDL.h>
#include <SDL_syswm.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace Parsers;


int main(int argc, char *argv[])
{
	auto arguments = gcnew List<String^>();
	for (int n = 1; n < argc; n++)  //argv[0] is the file path to the executable and not a parameter
	{
		arguments->Add(gcnew String(argv[n]));
	}
	CommandLine::Parse(arguments);
	Engine *engine = new Engine();
	engine->Init();
	return 0;
}