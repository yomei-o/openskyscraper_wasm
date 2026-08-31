#include "debugoptions.h"

using namespace OSS;


DebugOptions & DebugOptions::shared()
{
	//A function-local static so the options exist before main() has done
	//anything and no initialisation order matters.
	static DebugOptions options;
	return options;
}
