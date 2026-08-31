#include "video.h"

#include "../engine.h"
#include "../events/event.h"

using namespace OSS;





//----------------------------------------------------------------------------------------------------
#pragma mark -
#pragma mark Initialization
//----------------------------------------------------------------------------------------------------

Video::Video(Engine * engine) : engine(engine)
{
	//Initialize the video subsystem
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	//Prepare some OpenGL attributes
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, true);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	
	//Setup some default values
	safeMode.resolution = int2(1280, 768);
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
	softgl::set_target(surface->pixels, surface->w, surface->h, surface->pitch,
					   surface->format->Rshift, surface->format->Gshift,
					   surface->format->Bshift, surface->format->Ashift);
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
					   surface->format->Rshift, surface->format->Gshift,
					   surface->format->Bshift, surface->format->Ashift);
#else
	SDL_GL_SwapBuffers();
#endif
}
