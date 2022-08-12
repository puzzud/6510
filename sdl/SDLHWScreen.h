/*
 *  SDLHWScreen.h
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

#ifndef SDLHWSCREEN_H
#define SDLHWSCREEN_H

class SDLHWScreen : public CVICHWScreen {
public:
	SDLHWScreen(){}
	~SDLHWScreen(){}

	void Init();
	void Shutdown();
protected:
	SDL_Window* Window;
	SDL_Renderer* Renderer;
	SDL_Point RenderScale;

	void OnRasterLineCompleted(unsigned int lineNumber);
	void DrawChar(u16 address, u8 c);
	void DrawChars(u8* memory);

	void InitializeColors(void);

	void SetColorValuesFromInt(SDL_Color* color, unsigned int value);

	void DrawBufferOnLine(u16 screenXPosition, u8 screenYPosition, u8* pixelColorBuffer, unsigned int numberOfPixels);
	void DrawScreenLine(unsigned int lineNumber);
};

#endif //SDLHWSCREEN_H