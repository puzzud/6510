/*
 *  SDLHWController.cpp
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


const std::map<SDL_Keycode, CiaKeyboardMatrixPair> SdlKeyCodeToCiaKeyMatrixMap {
	{SDLK_DOWN, {0, 7}}, {SDLK_F5, {0, 6}}, {SDLK_F3, {0, 5}}, {SDLK_F1, {0, 4}}, {SDLK_F7, {0, 3}}, {SDLK_RIGHT, {0, 2}}, {SDLK_RETURN, {0, 1}}, {SDLK_BACKSPACE, {0, 0}},
	{SDLK_LSHIFT, {1, 7}}, {SDLK_e, {1, 6}}, {SDLK_s, {1, 5}}, {SDLK_z, {1, 4}}, {SDLK_4, {1, 3}}, {SDLK_a, {1, 2}}, {SDLK_w, {1, 1}}, {SDLK_3, {1, 0}},
	{SDLK_x, {2, 7}}, {SDLK_t, {2, 6}}, {SDLK_f, {2, 5}}, {SDLK_c, {2, 4}}, {SDLK_6, {2, 3}}, {SDLK_d, {2, 2}}, {SDLK_r, {2, 1}}, {SDLK_5, {2, 0}},
	{SDLK_v, {3, 7}}, {SDLK_u, {3, 6}}, {SDLK_h, {3, 5}}, {SDLK_b, {3, 4}}, {SDLK_8, {3, 3}}, {SDLK_g, {3, 2}}, {SDLK_y, {3, 1}}, {SDLK_7, {3, 0}},
	{SDLK_n, {4, 7}}, {SDLK_o, {4, 6}}, {SDLK_k, {4, 5}}, {SDLK_m, {4, 4}}, {SDLK_0, {4, 3}}, {SDLK_j, {4, 2}}, {SDLK_i, {4, 1}}, {SDLK_9, {4, 0}},
	{SDLK_COMMA, {5, 7}}, {SDLK_PAGEUP, {5, 6}}, {SDLK_SEMICOLON, {5, 5}}, {SDLK_PERIOD, {5, 4}}, {SDLK_MINUS, {5, 3}}, {SDLK_l, {5, 2}}, {SDLK_p, {5, 1}}, {SDLK_LEFTBRACKET, {5, 0}},
	{SDLK_SLASH, {6, 7}}, {SDLK_TAB, {6, 6}}, {SDLK_EQUALS, {6, 5}}, {SDLK_RSHIFT, {6, 4}}, {SDLK_HOME, {6, 3}}, {SDLK_QUOTE, {6, 2}}, {SDLK_RIGHTBRACKET, {6, 1}}, {SDLK_PAGEDOWN, {6, 0}},
	{SDLK_ESCAPE, {7, 7}}, {SDLK_q, {7, 6}}, {SDLK_LALT, {7, 5}}, {SDLK_SPACE, {7, 4}}, {SDLK_2, {7, 3}}, {SDLK_LCTRL, {7, 2}}, {SDLK_BACKQUOTE, {7, 1}}, {SDLK_1, {7, 0}}
};


void SDLHWController::Init(){
	SDL_InitSubSystem(SDL_INIT_EVENTS);

	keyboardMode = eKeyboardModeSymbolic;
}


void SDLHWController::Shutdown(){
	SDL_QuitSubSystem(SDL_INIT_EVENTS);
}


void SDLHWController::Process(bool& running){
	SDL_Event event;
	while (SDL_PollEvent(&event) != 0){
		switch (event.type){
			case SDL_QUIT:{
				running = false;

				break;
			}

			case SDL_KEYDOWN:
			case SDL_KEYUP:
            case SDL_TEXTINPUT:
			case SDL_CONTROLLERAXISMOTION:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:{
				OnInputEvent(&event);

				break;
			}
		}
	}
}


void SDLHWController::OnInputEvent(SDL_Event* event){
	switch (event->type){
		case SDL_KEYDOWN:{
			OnInputKeyEvent(event, 0);

			break;
		}

		case SDL_KEYUP:{
			OnInputKeyEvent(event, 1);

			break;
		}

        case SDL_TEXTINPUT:{
			OnInputTextInputEvent(event);

			break;
		}
	}
}


void SDLHWController::OnInputKeyEvent(SDL_Event* event, unsigned int isDown)
{
	switch (event->type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (event->key.repeat > 0)
			{
				return;
			}
			
			auto keyCode = event->key.keysym.sym;

			if (event->type == SDL_KEYDOWN && keyCode == SDLK_F9){
				// Increment keyboard mode.
				// NOTE: Keyset mode 3 (expecting BASIC)
				// is skipped in the cycle,
				// but can be the default.
				if (keyboardMode == eKeyboardModeBASIC){
					keyboardMode = eKeyboardModeSymbolic;
				}else{
					keyboardMode = (eKeyboardMode)((keyboardMode + 1) % 3);
				}

				cout << "Keyset Mode: " << keyboardMode << endl;
				return;
			}

			if (keyboardMode == eKeyboardModeBASIC){
				if (event->type != SDL_KEYDOWN){
					return;
				}
				
				int keyScanCode = event->key.keysym.scancode;

				//int keySymbol = event->key.keysym.sym;
				//cout << "Typed: " << int(keyScanCode) << " : " << char(keySymbol) << " (" << int(keySymbol) << ")" << endl;

				//https://sta.c64.org/cbm64pet.html
				
				int keyStroke = -1;

				if (keyScanCode == SDL_SCANCODE_RETURN)
				{
					keyStroke = 13;
				}
				else if (keyScanCode == SDL_SCANCODE_ESCAPE)
				{
					keyStroke = 3;
				}
				else if (keyScanCode == SDL_SCANCODE_UP)
				{
					keyStroke = 145;
				}
				else if (keyScanCode == SDL_SCANCODE_DOWN)
				{
					keyStroke = 17;
				}
				else if (keyScanCode == SDL_SCANCODE_LEFT)
				{
					keyStroke = 157;
				}
				else if (keyScanCode == SDL_SCANCODE_RIGHT)
				{
					keyStroke = 29;
				}
				else if (keyScanCode == SDL_SCANCODE_BACKSPACE)
				{
					keyStroke = 20;
				}

				if (keyStroke > -1)
				{
					cia1->AddKeyStroke(char(keyStroke));
				}

				return;
			}else if (keyboardMode == eKeyboardModeJoystick1 ||
				keyboardMode == eKeyboardModeJoystick2){
				int buttonId = -1;
				if (keyCode == SDLK_UP)
				{
					buttonId = 0;
				}
				else if (keyCode == SDLK_DOWN)
				{
					buttonId = 1;
				}
				else if (keyCode == SDLK_LEFT)
				{
					buttonId = 2;
				}
				else if (keyCode == SDLK_RIGHT)
				{
					buttonId = 3;
				}
				else if (keyCode == SDLK_SPACE)
				{
					buttonId = 4;
				}

				if (buttonId > -1)
				{
					cia1->SetJoystickState(keyboardMode - 1, buttonId, event->type == SDL_KEYDOWN);
					return;
				}
			}

			// Keyboard is keyboard (keyboardMode 0, keyboardModes 1 & 2 fall through behavior).
			auto it = SdlKeyCodeToCiaKeyMatrixMap.find(keyCode);
			if (it != SdlKeyCodeToCiaKeyMatrixMap.cend())
			{
				auto ciaKeyboardMatrixPair = &it->second;
				cia1->SetKeyState(ciaKeyboardMatrixPair->row, ciaKeyboardMatrixPair->column, event->type == SDL_KEYDOWN);
			}
		}

		break;
	}
}


void SDLHWController::OnInputTextInputEvent(SDL_Event* event)
{
	if (keyboardMode == eKeyboardModeBASIC){
		char keySymbol = event->text.text[0];
		//cout << "Text Input: " << char(keySymbol) << endl;

		if (keySymbol >= 'a' && keySymbol <= 'z')
		{
			keySymbol = keySymbol + 'A' - 'a';
		}

		cia1->AddKeyStroke(char(keySymbol));
	}
}
