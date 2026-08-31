#include <cstring>

#include "engine/base/autoreleasequeue.h"
#include "game/application.h"
#include "game/debugoptions.h"

using namespace OSS;


int main(int argc, char *argv[])
{
	//--debug turns on everything that makes the game easier to look at.  The
	//individual switches are there so a session can have one without the other.
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--debug")) {
			DebugOptions::shared().unlimitedFunds = true;
			DebugOptions::shared().maxRating = true;
		}
		else if (!strcmp(argv[i], "--unlimited-funds"))
			DebugOptions::shared().unlimitedFunds = true;
		else if (!strcmp(argv[i], "--max-rating"))
			DebugOptions::shared().maxRating = true;
	}

	AutoreleaseQueue queue;
	
	OpenSkyscraper * app = new OpenSkyscraper;
	app->run();
	
	return 0;
}
