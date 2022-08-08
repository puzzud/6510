#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../A64Core/CBM64Main.h"
#include "../A64Core/General.h"


#define FRAMES_PER_SECOND	60
#define USE_PERFORMANCE_COUNTER 1


void Process(void);
void MainLoop(void);
void MainLoopIteration(void);

// TODO: Make these functions not extern.
extern void ProcessSdlEvents();


extern CBM64Main* cbm64;

bool running;
uint64_t remainingCycles = 0;


void MainLoop(void){
	running = true;

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoopIteration, FRAMES_PER_SECOND, 1);
	return;
#else
	while (running){
		MainLoopIteration();
	}
#endif
}


inline void MainLoopIteration(void){
#ifndef __EMSCRIPTEN__
#ifndef USE_PERFORMANCE_COUNTER
	Uint32 FrameStart = SDL_GetTicks();
#else
	Uint64 FrameStart = SDL_GetPerformanceCounter();
#endif
#endif

	ProcessSdlEvents();

	Process();

#ifndef __EMSCRIPTEN__
	// Cap to 60 FPS.

#ifndef USE_PERFORMANCE_COUNTER
	Uint32 FrameTime = SDL_GetTicks();
	float elapsedTimeMs = FrameTime - FrameStart;
#else
	Uint64 FrameTime = SDL_GetPerformanceCounter();
	float elapsedTimeMs = (FrameTime - FrameStart) / (float)SDL_GetPerformanceFrequency() * 1000.0f;
#endif

	float delayTimeMs = floor((1000.0f / (float)FRAMES_PER_SECOND) - elapsedTimeMs);
	if (delayTimeMs > 0.0){
		SDL_Delay(delayTimeMs);
	}

#endif
}


void Process(void){
	//1023000 // NTSC
	static const int cyclesInFrame = NTSC_FIELD_CYCLES_PER_LINE * NTSC_FIELD_LINE_HEIGHT;

	int cycles = remainingCycles;

	while (true){
		cycles += cbm64->Cycle();
		if (cycles >= cyclesInFrame){
			remainingCycles = cycles - cyclesInFrame;
			break;
		}
	}
}
