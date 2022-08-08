#include <chrono>
#include <ctime>
#include <atomic>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <map>

#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../A64Core/CBM64Main.h"
#include "../A64Core/General.h"

#include "SDLHWScreen.h"

#define FRAMES_PER_SECOND	60
#define USE_PERFORMANCE_COUNTER 1

typedef enum _eKeyboardMode{
	eKeyboardModeSymbolic,
	eKeyboardModeJoystick1,
	eKeyboardModeJoystick2,
	eKeyboardModeBASIC
}eKeyboardMode;

void Process(void);
void MainLoop(void);
void MainLoopIteration(void);

void OnInputEvent(SDL_Event* event);
void OnInputKeyEvent(SDL_Event* event, unsigned int isDown);
void OnInputTextInputEvent(SDL_Event* event);


CBM64Main* cbm64 = NULL;

eKeyboardMode keysetMode = eKeyboardModeBASIC;

bool running = true;
uint64_t remainingCycles = 0;

typedef struct _CiaKeyboardMatrixPair{
	u8 row;
	u8 column;
} CiaKeyboardMatrixPair;

std::map<SDL_Keycode, CiaKeyboardMatrixPair> SdlKeyCodeToCiaKeyMatrixMap {
	{SDLK_DOWN, {0, 7}}, {SDLK_F5, {0, 6}}, {SDLK_F3, {0, 5}}, {SDLK_F1, {0, 4}}, {SDLK_F7, {0, 3}}, {SDLK_RIGHT, {0, 2}}, {SDLK_RETURN, {0, 1}}, {SDLK_BACKSPACE, {0, 0}},
	{SDLK_LSHIFT, {1, 7}}, {SDLK_e, {1, 6}}, {SDLK_s, {1, 5}}, {SDLK_z, {1, 4}}, {SDLK_4, {1, 3}}, {SDLK_a, {1, 2}}, {SDLK_w, {1, 1}}, {SDLK_3, {1, 0}},
	{SDLK_x, {2, 7}}, {SDLK_t, {2, 6}}, {SDLK_f, {2, 5}}, {SDLK_c, {2, 4}}, {SDLK_6, {2, 3}}, {SDLK_d, {2, 2}}, {SDLK_r, {2, 1}}, {SDLK_5, {2, 0}},
	{SDLK_v, {3, 7}}, {SDLK_u, {3, 6}}, {SDLK_h, {3, 5}}, {SDLK_b, {3, 4}}, {SDLK_8, {3, 3}}, {SDLK_g, {3, 2}}, {SDLK_y, {3, 1}}, {SDLK_7, {3, 0}},
	{SDLK_n, {4, 7}}, {SDLK_o, {4, 6}}, {SDLK_k, {4, 5}}, {SDLK_m, {4, 4}}, {SDLK_0, {4, 3}}, {SDLK_j, {4, 2}}, {SDLK_i, {4, 1}}, {SDLK_9, {4, 0}},
	{SDLK_COMMA, {5, 7}}, {SDLK_PAGEUP, {5, 6}}, {SDLK_SEMICOLON, {5, 5}}, {SDLK_PERIOD, {5, 4}}, {SDLK_MINUS, {5, 3}}, {SDLK_l, {5, 2}}, {SDLK_p, {5, 1}}, {SDLK_LEFTBRACKET, {5, 0}},
	{SDLK_SLASH, {6, 7}}, {SDLK_TAB, {6, 6}}, {SDLK_EQUALS, {6, 5}}, {SDLK_RSHIFT, {6, 4}}, {SDLK_HOME, {6, 3}}, {SDLK_QUOTE, {6, 2}}, {SDLK_RIGHTBRACKET, {6, 1}}, {SDLK_PAGEDOWN, {6, 0}},
	{SDLK_ESCAPE, {7, 7}}, {SDLK_q, {7, 6}}, {SDLK_LALT, {7, 5}}, {SDLK_SPACE, {7, 4}}, {SDLK_2, {7, 3}}, {SDLK_LCTRL, {7, 2}}, {SDLK_BACKQUOTE, {7, 1}}, {SDLK_1, {7, 0}}
};


void Process(void){
	//1023000 // NTSC
	static const int cyclesInFrame = NTSC_FIELD_CYCLES_PER_LINE * NTSC_FIELD_LINE_HEIGHT;

	int cycles = remainingCycles;

	while (true)
	{
		cycles += cbm64->Cycle();
		if (cycles >= cyclesInFrame)
		{
			remainingCycles = cycles - cyclesInFrame;
			break;
		}
	}
}


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_EVENTS);

    cbm64 = new CBM64Main();
    cbm64->Init();

    //Subscribe Host hardware related VIC Code
    SDLHWScreen hwScreen;
    cbm64->GetVic()->RegisterHWScreen(&hwScreen);
	hwScreen.Init();

	if (argc > 1)
	{
		if (cbm64->LoadAppWithoutBasic(argv[1]) == 0){
			keysetMode = eKeyboardModeSymbolic;
		}
	}

	if (argc > 2)
	{
		cbm64->GetCpu()->SetPC(std::strtol(argv[2], NULL, 16));
	}

#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(MainLoopIteration, FRAMES_PER_SECOND, 1);
	return 0;
#else
	MainLoop();

	hwScreen.Shutdown();
    SDL_Quit();

	std::cout << "ended..." << std::endl;

	return 0;
#endif
}

inline void MainLoop(void)
{
	while (running)
	{
		MainLoopIteration();
	}
}

inline void MainLoopIteration(void)
{
#ifndef __EMSCRIPTEN__
#ifndef USE_PERFORMANCE_COUNTER
	Uint32 FrameStart = SDL_GetTicks();
#else
	Uint64 FrameStart = SDL_GetPerformanceCounter();
#endif
#endif

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				running = false;

				break;
			}

			case SDL_KEYDOWN:
			case SDL_KEYUP:
            case SDL_TEXTINPUT:
			case SDL_CONTROLLERAXISMOTION:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			{
				OnInputEvent(&event);

				break;
			}
		}
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
	if (delayTimeMs > 0.0)
	{
		SDL_Delay(delayTimeMs);
	}

#endif
}

void OnInputEvent(SDL_Event* event)
{
	switch (event->type)
	{
		case SDL_KEYDOWN:
		{
			OnInputKeyEvent(event, 0);

			break;
		}

		case SDL_KEYUP:
		{
			OnInputKeyEvent(event, 1);

			break;
		}

        case SDL_TEXTINPUT:
		{
			OnInputTextInputEvent(event);

			break;
		}
	}
}

void OnInputKeyEvent(SDL_Event* event, unsigned int isDown)
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
				// Increment keyset mode.
				// NOTE: Keyset mode 3 (expecting BASIC)
				// is skipped in the cycle,
				// but can be the default.
				if (keysetMode == eKeyboardModeBASIC){
					keysetMode = eKeyboardModeSymbolic;
				}else{
					keysetMode = (eKeyboardMode)((keysetMode + 1) % 3);
				}

				cout << "Keyset Mode: " << keysetMode << endl;
				return;
			}

			if (keysetMode == eKeyboardModeBASIC){
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
					cbm64->GetCia1()->AddKeyStroke(char(keyStroke));
				}

				return;
			}else if (keysetMode == eKeyboardModeJoystick1 ||
				keysetMode == eKeyboardModeJoystick2){
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
					cbm64->GetCia1()->SetJoystickState(keysetMode - 1, buttonId, event->type == SDL_KEYDOWN);
					return;
				}
			}

			// Keyboard is keyboard (keysetMode 0, keysetModes 1 & 2 fall through behavior).
			auto it = SdlKeyCodeToCiaKeyMatrixMap.find(keyCode);
			if (it != SdlKeyCodeToCiaKeyMatrixMap.cend())
			{
				auto ciaKeyboardMatrixPair = &it->second;
				cbm64->GetCia1()->SetKeyState(ciaKeyboardMatrixPair->row, ciaKeyboardMatrixPair->column, event->type == SDL_KEYDOWN);
			}
		}

		break;
	}
}

void OnInputTextInputEvent(SDL_Event* event)
{
	if (keysetMode == 3){
		char keySymbol = event->text.text[0];
		//cout << "Text Input: " << char(keySymbol) << endl;

		if (keySymbol >= 'a' && keySymbol <= 'z')
		{
			keySymbol = keySymbol + 'A' - 'a';
		}

		cbm64->GetCia1()->AddKeyStroke(char(keySymbol));
	}
}
