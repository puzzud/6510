/*
 *  SDLHWController.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 07/06/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <map>
 
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../A64Core/CBM64Main.h"
#include "../A64Core/General.h"

#ifndef SDLHWCONTROLLER_H
#define SDLHWCONTROLLER_H


typedef struct _CiaKeyboardMatrixPair{
	u8 row;
	u8 column;
} CiaKeyboardMatrixPair;


typedef enum _eKeyboardMode{
	eKeyboardModeSymbolic,
	eKeyboardModeJoystick1,
	eKeyboardModeJoystick2,
	eKeyboardModeBASIC
}eKeyboardMode;


class SDLHWController : public CCIA1HWController {
public:
	SDLHWController(){}
	~SDLHWController(){}

	void Init();
	void Shutdown();

	void Process(bool& running);

	eKeyboardMode keyboardMode;
protected:
	void OnInputEvent(SDL_Event* event);
	void OnInputKeyEvent(SDL_Event* event, unsigned int isDown);
	void OnInputTextInputEvent(SDL_Event* event);
};

#endif //SDLHWCONTROLLER_H