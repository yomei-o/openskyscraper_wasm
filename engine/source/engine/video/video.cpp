#include "video.h"

#include "../engine.h"
#include "../events/event.h"

using namespace OSS;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

//The browser window in CSS pixels, which is what the canvas element is
//sized to.  Clamped so a collapsed window cannot produce a zero-sized
//surface.
static int2 browserResolution()
{
	int w = (int)emscripten_run_script_int("window.innerWidth");
	int h = (int)emscripten_run_script_int("window.innerHeight");
	return int2(std::max(w, 320), std::max(h, 240));
}

void Video::checkBrowserSize()
{
	int2 size = browserResolution();
	if (size == currentMode.resolution) return;
	desiredMode.resolution = size;
	activateMode();
}
#endif

Video::Video(Engine * engine) : engine(engine)
{
	//Initialize the video subsystem.
	//
	//emscripten needs SDL_Init here, not SDL_InitSubSystem: its
	//SDL_InitSubSystem is literally `(flags) => 0`, while SDL_Init is what
	//builds the DOM-event-name -> SDL-event-type table and allocates the
	//keyboard state.  Without it every event that reaches SDL_PollEvent gets
	//its type written from an undefined lookup, i.e. 0, so mouse buttons and
	//keys arrive as events of no known kind and nothing ever handles them.
#ifdef __EMSCRIPTEN__
	SDL_Init(SDL_INIT_VIDEO);
#else
	SDL_InitSubSystem(SDL_INIT_VIDEO);
#endif
	
	//Prepare some OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	
	//Setup some default values
#ifdef __EMSCRIPTEN__
	//Render at exactly the browser window's size.  Drawing at a fixed
	//resolution and letting CSS scale the canvas costs both legibility and
	//accuracy: the resampling destroys the bitmap text, and emscripten maps
	//pointer coordinates through canvas.width / rect.width, so any scale
	//other than 1 misplaces clicks by an amount that grows across the
	//window.  At 1:1 both problems are gone by construction.
	safeMode.resolution = browserResolution();
#else
	safeMode.resolution = int2(1280, 768);
#endif
	safeMode.fullscreen = false;
	desiredMode = safeMode;
	currentMode = safeMode;
	
	//Activate the desired mode
	activateMode();
}

Video::~Video()
{
	//Shut down the video subsystem
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Video Mode
//----------------------------------------------------------------------------------------------------

bool Video::activateMode()
{
	bool success = switchToMode(&desiredMode);
	if (success) {
		//Store the current mode
		SDL_Surface * surface = SDL_GetVideoSurface();
		currentMode.resolution.x = surface->w;
		currentMode.resolution.y = surface->h;
		currentMode.fullscreen = (surface->flags & SDL_FULLSCREEN);
		
		//Issue an event to inform the game about the mode change
		Pointer<VideoEvent> e = new VideoEvent(Event::kVideoChanged, this);
		engine->application->sendEvent(e);
	}
	return success;
}

void Video::revertToSafeMode()
{
	assert(switchToMode(&safeMode));
}

void Video::confirmCurrentModeToBeSafe()
{
	safeMode = currentMode;
}

bool Video::switchToMode(VideoMode * mode)
{
	if (!mode) return false;
	
#ifdef __EMSCRIPTEN__
	//There is no WebGL here, so the surface is a plain software one and the
	//fixed-function GL calls are rasterised into it by compat/softgl.
	Uint32 flags = SDL_SWSURFACE;
	SDL_Surface * surface = SDL_SetVideoMode(mode->resolution.x,
											 mode->resolution.y, 32, flags);
	if (!surface) return false;
	//SDL_LockSurface is what allocates the pixel buffer and fills in
	//surface->pixels; before the first lock it is null.  The surface stays
	//locked while drawing and is unlocked once per frame to present.
	SDL_LockSurface(surface);
	//Not surface->format: emscripten's SDL_UnlockSurface copies the surface
	//32 bits at a time into an ImageData, which is then read as R,G,B,A bytes.
	//The buffer has to be in canvas byte order whatever the format claims.
	softgl::set_target(surface->pixels, surface->w, surface->h, surface->pitch,
					   0, 8, 16, 24);
	return true;
#else
	Uint32 flags = SDL_OPENGL;
	if (mode->fullscreen)
		flags |= SDL_FULLSCREEN;
	else
		flags |= SDL_RESIZABLE;
	
	return (SDL_SetVideoMode(mode->resolution.x, mode->resolution.y, 0, flags) != NULL);
#endif
}

void Video::swapBuffers()
{
#ifdef __EMSCRIPTEN__
	//SDL_Flip does nothing here: emscripten's SDL 1.2 copies the surface to the
	//canvas inside SDL_UnlockSurface.  Unlock to present, then lock again so
	//the next frame has somewhere to draw.
	SDL_Surface * surface = SDL_GetVideoSurface();
	if (!surface) return;
	SDL_UnlockSurface(surface);
	SDL_LockSurface(surface);
	softgl::set_target(surface->pixels, surface->w, surface->h, surface->pitch,
					   0, 8, 16, 24);
#else
	SDL_GL_SwapBuffers();
#endif
}
