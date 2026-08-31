#include "application.h"

using namespace OSS;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

Application::Application()
{	
	//Initialize DevIL
	ilInit();
	ilEnable(IL_FILE_OVERWRITE);
	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
	
	//Fire up the engine!
	engine = new Engine(this);
	
	//Make sure we received keyboard repeat events
	SDL_EnableKeyRepeat(250, 50);
	
	//Supporting Unicode wouldn't hurt either in the 21st centurey
	SDL_EnableUNICODE(SDL_TRUE);
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Run Loop
//----------------------------------------------------------------------------------------------------

//The run loop gets its own autorelease queue.  It is a file static rather than
//a local so that the Emscripten build, whose frames arrive one browser callback
//at a time, can share it with runLoopIteration().
static AutoreleaseQueue * runLoopGarbage = NULL;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
static Application * emscriptenApp = NULL;
static void emscriptenFrame()
{
	if (emscriptenApp) emscriptenApp->runLoopIteration();
}
#endif

void Application::runLoopIteration()
{
	//Notify
	willIterateRunLoop();
	
	//Send the events from the pump down the responder chain
	pumpEvents();
	
	//If we're supposed to terminate, do so
	if (terminateReply == kTerminateLater)
		terminate();
	
	//TODO: advance the engine
	engine->heartbeat();
	
	//Notify
	didIterateRunLoop();
	
	//Get rid of the garbage
	if (runLoopGarbage) runLoopGarbage->drain();
}

void Application::run()
{
	terminateReply = kTerminateCancel;
	willRun();
	
	runLoopGarbage = new AutoreleaseQueue;
	
#ifdef __EMSCRIPTEN__
	//Hand the frame clock to the browser.  This does not return: it unwinds
	//the stack and calls emscriptenFrame() once per animation frame.
	emscriptenApp = this;
	emscripten_set_main_loop(emscriptenFrame, 0, 1);
#else
	while (terminateReply != kTerminateNow)
		runLoopIteration();
	
	//Get rid of the garbage queue
	delete runLoopGarbage;
	runLoopGarbage = NULL;
	
	didRun();
#endif
}

bool Application::isRunning()
{
	return (terminateReply != kTerminateNow);
}



void Application::terminate()
{
	terminateReply = shouldTerminate();
}

bool Application::isTerminating()
{
	return (terminateReply != kTerminateCancel);
}

Application::TerminateReply Application::shouldTerminate()
{
	return kTerminateNow;
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Events
//----------------------------------------------------------------------------------------------------

Event * Application::getNextEvent()
{
	//Fetch the next SDL event
	SDL_Event event;
	if (!SDL_PollEvent(&event))
		return NULL;
	
	//A variable for the interpreted event
	Event * e = NULL;
	
	//Fetch the window size so we can flip the mouse event
	int h = Video::shared()->currentMode.resolution.y;
	
	//Interpret mouse button events
	if (SDL_EVENTMASK(event.type) & (SDL_MOUSEBUTTONDOWNMASK | SDL_MOUSEBUTTONUPMASK)) {
		e = new MouseButtonEvent(int2(event.button.x, h - event.button.y),
								 event.button.button,
								 (event.button.state == SDL_PRESSED));
	}
	
	//Interpret mouse moved events
	if (SDL_EVENTMASK(event.type) & (SDL_MOUSEMOTIONMASK)) {
		e = new MouseMoveEvent(int2(event.motion.x, h - event.motion.y),
								int2(event.motion.xrel, -event.motion.yrel));
	}
	
	//TODO: Create some sort of mouse dragged events
	
	//Interpret scroll wheel events
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (event.button.button == SDL_BUTTON_WHEELUP)
			e = new ScrollWheelEvent(int2(event.button.x, h - event.button.y), double2(0, 1));
		if (event.button.button == SDL_BUTTON_WHEELDOWN)
			e = new ScrollWheelEvent(int2(event.button.x, h - event.button.y), double2(0, -1));
	}
	
	//Interpret key events
	if (SDL_EVENTMASK(event.type) & (SDL_KEYDOWNMASK | SDL_KEYUPMASK)) {
		e = new KeyEvent(event.key.keysym.unicode, event.key.keysym.sym,
						 (event.key.state == SDL_PRESSED),
						 false);
	}
	
	//Wrap up unhandled SDL events
	if (!e)
		e = new SDLEvent(event);
	
	//Special treatment for quit events
	if (event.type == SDL_QUIT)
		terminate();
	
	//Special shortcut for quitting
	if (event.type == SDL_KEYDOWN && event.key.keysym.mod & KMOD_META &&
		event.key.keysym.unicode == 'q')
		terminate();
	
	return e;
}

void Application::pumpEvents()
{
	willPumpEvents();
	
	Event * event;
	while ((event = getNextEvent()))
		sendEvent(event);
	
	didPumpEvents();
}

bool Application::sendEventToNextResponders(Event * event)
{
	if (engine && engine->sendEvent(event)) return true;
	return BasicResponder::sendEventToNextResponders(event);
}




//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Resources
//----------------------------------------------------------------------------------------------------

string Application::pathToResource(string resourceGroup, string resourceName, string resourceType)
{
	return pathToResource(resourceGroup, resourceName + "." + resourceType);
}
