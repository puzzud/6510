/*
 *  SDLMainLoop.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 07/06/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../A64Core/CBM64Main.h"
#include "../A64Core/General.h"

#include "SDLHWController.h"

#define FRAMES_PER_SECOND	60
#define USE_PERFORMANCE_COUNTER 1


void MainLoop(void);
void MainLoopIteration(void);
void Process(void);


extern CBM64Main* cbm64;

bool running;
uint64_t remainingCycles = 0;


void MainLoop(void){
	running = true;

	CCIA1HWController* hwController = cbm64->GetCia1()->GetHWController();
	if (hwController != NULL){
		SDLHWController* sdlHwController = dynamic_cast<SDLHWController*>(hwController);
		if (sdlHwController != NULL){
			// TODO: Would be great to determine when to use this mode based on
			// to presence of BASIC and other factors--may be hard to do it
			// faithfully. Perhaps check 6510 IO for both BASIC & kernal
			// and some other things like the interrupt vector values and
			// and the raster compare line number on the VIC. But checking that
			// that stuff right now is too early.			
			sdlHwController->keyboardMode = eKeyboardModeBASIC;
		}
	}

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

	CCIA1HWController* hwController = cbm64->GetCia1()->GetHWController();
	if (hwController != NULL){

		hwController->Process(running);
	}

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
	auto vic = cbm64->GetVic();

	int cycles = remainingCycles;

	while (true){
		cycles += cbm64->Cycle();
		if (cycles >= vic->GetCyclesPerFrame()){
			remainingCycles = cycles - vic->GetCyclesPerFrame();
			break;
		}
	}
}
