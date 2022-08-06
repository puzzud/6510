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

#define FRAMES_PER_SECOND	60
#define USE_PERFORMANCE_COUNTER 1

typedef enum
{
	COLOR_BLACK,
	COLOR_WHITE,
	COLOR_RED,
	COLOR_CYAN,
	COLOR_PURPLE,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_YELLOW,
	COLOR_ORANGE,
	COLOR_BROWN,
	COLOR_LIGHT_RED,
	COLOR_GREY_1,
	COLOR_GREY_2,
	COLOR_LIGHT_GREEN,
	COLOR_LIGHT_BLUE,
	COLOR_GREY_3,

	NUMBER_OF_COLORS
} ColorCode;

void Process(void);
void MainLoop(void);
void MainLoopIteration(void);
void OnInputEvent(SDL_Event* event);
void OnInputKeyEvent(SDL_Event* event, unsigned int isDown);
void OnInputTextInputEvent(SDL_Event* event);
void InitializeColors(void);
void SetColorValuesFromInt(SDL_Color* color, unsigned int value);
void DrawScreen();
void DrawByte(u8 byte, u8* colorCodes, u16 screenXPosition, u8 screenYPosition, int mode, bool multiColor = false, unsigned int horizontalScale = 1);
void DrawBufferOnLine(u16 screenXPosition, u8 screenYPosition, u8* pixelColorBuffer, unsigned int numberOfPixels);
void DrawScreenLine(unsigned int lineNumber);

class CustomWatcher;
class HiresTimeImpl;
class EMCScreen;

CBM64Main* cbm64 = NULL;
EMCScreen* emcScreen_ = NULL;
CustomWatcher* watcher = NULL;

SDL_Window* Window;
SDL_Renderer* Renderer;
SDL_Point RenderScale;
SDL_Color Colors[NUMBER_OF_COLORS];
SDL_Texture* CharacterSetTexture;

unsigned int keysetMode = 0;

bool _run = true;
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


class CustomWatcher : public CWatcher
{
    public:
	protected:
	
	virtual void ReportJumpWatch(u16 address, eWatcherJumpType jumpType)
	{
		cout << "Jump: " << std::hex << int(address) << std::dec << endl;

		if (address == 0x655B)
		{
			if (cbm64->LoadAppWithoutBasic("/home/puzzud/temp/Desktop/m") == 0)
			{
				cout << "Loaded M" << endl;
				cbm64->GetCpu()->SetPC(0x4000);
				
			}
		}
	};

	virtual void ReportReadWatch(u16 address)
	{
		cout << "Read: " << std::hex << int(address) << std::dec << endl;
	}

	virtual void ReportWriteWatch(u16 address)
	{
		cout << "Write: " << std::hex << int(address) << std::dec << endl;
	}

    private:
};


uint64_t now() {
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return now;
}


class HiresTimeImpl : public CHiresTime {
    public:
        HiresTimeImpl(){
            start_ = std::chrono::high_resolution_clock::now();
        }
    public:
        double GetMicroseconds() {
            auto t2 = std::chrono::high_resolution_clock::now();

            auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - start_);
            std::chrono::duration<long, std::micro> int_usec = int_ms;

            return int_usec.count();
        }

        int GetMicrosecondsLo() {
            static timeval t;
            gettimeofday(&t, NULL);	
            return ((t.tv_sec * 1000000) + t.tv_usec);
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};


class EMCScreen : public CVICHWScreen {
    public:
        EMCScreen() {
			// TODO: Maybe best to just pull this data directly from VIC,
			// instead of duplicating in HWScreen.
			SetBorderColor(0);
			SetBackgroundColor(0);
        }
        ~EMCScreen(){}
    public:
        void DrawChar(u16 address, u8 c){}
        void DrawChars(u8* memory){}
		void OnRasterLineCompleted(unsigned int lineNumber){
			// TODO: Catch & translate border lines & V blank lines.
			DrawScreenLine(lineNumber);

			if (lineNumber == NTSC_FIELD_LINE_HEIGHT){
				SDL_RenderPresent(Renderer);
				
				// Clear render display (avoids artifacts in "border" in SDL window when resized).
				//SDL_Color* color = &Colors[emcScreen_->GetBorderColor()];
				SDL_Color* color = &Colors[COLOR_BLACK];

				SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, 0xff);
				SDL_RenderClear(Renderer);
			}
		};
};


void runloop() {
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
    SDL_Init(SDL_INIT_VIDEO);
    RenderScale.x = 3;
	RenderScale.y = 3;
    Window = SDL_CreateWindow
		(
			"6510",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			SCREEN_WIDTH * RenderScale.x, SCREEN_HEIGHT * RenderScale.y,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
		);
    Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetScale(Renderer, (float)RenderScale.x, (float)RenderScale.y);
	SDL_RenderSetLogicalSize(Renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    InitializeColors();
    
    cbm64 = new CBM64Main();
    cbm64->Init();

    //Set the hires time callbacks
    HiresTimeImpl hiresTime;
    cbm64->SetHiresTimeProvider(&hiresTime);

    //Subscribe Host hardware related VIC Code
    emcScreen_ = new EMCScreen();
    cbm64->GetVic()->RegisterHWScreen(emcScreen_);

	watcher = new CustomWatcher();
	cbm64->SetWatcher(watcher);

	if (argc > 1)
	{
		cbm64->LoadAppWithoutBasic(argv[1]);

		watcher->SetJumpWatch(0x655B);
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

	std::cout << "ended..." << std::endl;

    SDL_DestroyWindow(Window);
	SDL_DestroyRenderer(Renderer);
    SDL_Quit();

	delete emcScreen_;
	emcScreen_ = NULL;

	delete watcher;
	watcher = NULL;

	return 0;
#endif
}

void Process(void)
{
    runloop();
}

inline void MainLoop(void)
{
	while (_run)
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
				_run = false;

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
	/*
	switch (event->type)
	{
		case SDL_KEYDOWN:
		{
			if (event->key.repeat == 0)
			{
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
			}

			break;
		}

		case SDL_KEYUP:
		{
			if (event->key.repeat == 0)
			{

			}

			break;
		}
	}
	*/

	switch (event->type)
	{
		case SDL_KEYDOWN:
		case SDL_KEYUP:
		{
			if (event->key.repeat == 0)
			{
				auto keyCode = event->key.keysym.sym;

				if (event->type == SDL_KEYDOWN && keyCode == SDLK_F9){
					// Increment keyset mode.
					keysetMode = (keysetMode + 1) % 3;
					cout << "Keyset Mode: " << keysetMode << endl;
					return;
				}

				if (keysetMode == 1 || keysetMode == 2)
				{
					// Keyboard controls joystick 1 or 2.
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

				// Keyboard is keyboard (keysetMode 0 and fall through behavior).
				auto it = SdlKeyCodeToCiaKeyMatrixMap.find(keyCode);
				if (it != SdlKeyCodeToCiaKeyMatrixMap.cend())
				{
					auto ciaKeyboardMatrixPair = &it->second;
					ciaKeyboardMatrixPair->row;
					ciaKeyboardMatrixPair->column;
					cbm64->GetCia1()->SetKeyState(ciaKeyboardMatrixPair->row, ciaKeyboardMatrixPair->column, event->type == SDL_KEYDOWN);
				}
			}

			break;
		}
	}
}

void OnInputTextInputEvent(SDL_Event* event)
{
	/*
    char keySymbol = event->text.text[0];
    //cout << "Text Input: " << char(keySymbol) << endl;

    char keyStroke = keySymbol;

    if (keySymbol >= 'a' && keySymbol <= 'z')
    {
        keyStroke = keySymbol + 'A' - 'a';
    }

    //cbm64->GetCia1()->AddKeyStroke(char(keyStroke));
	*/
}

void InitializeColors(void)
{
#define RGB_COLOR_BLACK       0x000000
#define RGB_COLOR_WHITE       0xffffff
#define RGB_COLOR_RED         0x880000
#define RGB_COLOR_CYAN        0xaaffee
#define RGB_COLOR_PURPLE      0xcc44cc
#define RGB_COLOR_GREEN       0x00cc55
#define RGB_COLOR_BLUE        0x0000aa
#define RGB_COLOR_YELLOW      0xeeee77
#define RGB_COLOR_ORANGE      0xdd8855
#define RGB_COLOR_BROWN       0x664400
#define RGB_COLOR_LIGHT_RED   0xff7777
#define RGB_COLOR_GREY_1      0x333333
#define RGB_COLOR_GREY_2      0x777777
#define RGB_COLOR_LIGHT_GREEN 0xaaff66
#define RGB_COLOR_LIGHT_BLUE  0x0088ff
#define RGB_COLOR_GREY_3      0xbbbbbb

	SetColorValuesFromInt(&Colors[COLOR_BLACK], RGB_COLOR_BLACK);
	SetColorValuesFromInt(&Colors[COLOR_WHITE], RGB_COLOR_WHITE);
	SetColorValuesFromInt(&Colors[COLOR_RED], RGB_COLOR_RED);
	SetColorValuesFromInt(&Colors[COLOR_CYAN], RGB_COLOR_CYAN);
	SetColorValuesFromInt(&Colors[COLOR_PURPLE], RGB_COLOR_PURPLE);
	SetColorValuesFromInt(&Colors[COLOR_GREEN], RGB_COLOR_GREEN);
	SetColorValuesFromInt(&Colors[COLOR_BLUE], RGB_COLOR_BLUE);
	SetColorValuesFromInt(&Colors[COLOR_YELLOW], RGB_COLOR_YELLOW);
	SetColorValuesFromInt(&Colors[COLOR_ORANGE], RGB_COLOR_ORANGE);
	SetColorValuesFromInt(&Colors[COLOR_BROWN], RGB_COLOR_BROWN);
	SetColorValuesFromInt(&Colors[COLOR_LIGHT_RED], RGB_COLOR_LIGHT_RED);
	SetColorValuesFromInt(&Colors[COLOR_GREY_1], RGB_COLOR_GREY_1);
	SetColorValuesFromInt(&Colors[COLOR_GREY_2], RGB_COLOR_GREY_2);
	SetColorValuesFromInt(&Colors[COLOR_LIGHT_GREEN], RGB_COLOR_LIGHT_GREEN);
	SetColorValuesFromInt(&Colors[COLOR_LIGHT_BLUE], RGB_COLOR_LIGHT_BLUE);
	SetColorValuesFromInt(&Colors[COLOR_GREY_3], RGB_COLOR_GREY_3);
}

void SetColorValuesFromInt(SDL_Color* color, unsigned int value)
{
	color->r = (value >> 16) & 0xff;
	color->g = (value >> 8) & 0xff;
	color->b = value & 0xff;
	color->a = 0xff;
}

void DrawScreen()
{
	for (unsigned int lineNumber = 0; lineNumber < SCREEN_HEIGHT; ++lineNumber)
	{
		DrawScreenLine(lineNumber);
	}
}

// mode parameter indicates character or sprite mode,
// because of the discrepancy between how bit pairs
// map to color values respectively.
//  - 0: Character
//  - 1: Sprite
void DrawByte(u8 byte, u8* colorCodes, u16 screenXPosition, u8 screenYPosition, int mode, bool multiColor, unsigned int horizontalScale)
{
	static u8 pixelColorBuffer[16];  // 16 bytes in case of horizontal scaling.
	memset(pixelColorBuffer, 0, 16);
	cbm64->GetVic()->DrawByteToBuffer(byte, pixelColorBuffer, colorCodes, mode, multiColor, horizontalScale);
	DrawBufferOnLine(screenXPosition, screenYPosition, pixelColorBuffer, 8 * horizontalScale);
}

void DrawBufferOnLine(u16 screenXPosition, u8 screenYPosition, u8* pixelColorBuffer, unsigned int numberOfPixels)
{
	// Draw to renderer from pixel buffers.
	SDL_Rect rect;
	rect.x = screenXPosition;
	rect.y = screenYPosition;
	rect.w = 1;
	rect.h = 1;

	for (int x = 0; x < numberOfPixels; ++x, ++rect.x)
	{
		if (pixelColorBuffer[x] == 0)
		{
			continue;
		}

		// Minus 1 to account for using 0 for transparency.
		u8 colorCode = pixelColorBuffer[x] - 1;
		SDL_Color* color = &Colors[colorCode & 0x0F];
		SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, color->a);
		SDL_RenderFillRect(Renderer, &rect);
	}
}

void DrawScreenLine(unsigned int lineNumber)
{
	// TODO: Figure out this number!
	int screenLineNumber = lineNumber - HARDWARE_SPRITE_TO_SCREEN_Y_OFFSET;
	if (screenLineNumber < 0)
	{
		return;
	}

	if (screenLineNumber >= SCREEN_HEIGHT)
	{
		return;
	}

	auto vic = cbm64->GetVic();

	// Draw background.
	// TODO: Does not account for background color in extended color mode.
	SDL_Rect rect;
	rect.y = screenLineNumber;
	rect.h = 1;

	rect.x = 0;
	rect.w = SCREEN_WIDTH;

	SDL_Color* backgroundColor = &Colors[emcScreen_->GetBackgroundColor() % 16];
	SDL_SetRenderDrawColor(Renderer, backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
	SDL_RenderFillRect(Renderer, &rect);

	// Draw background.
	static u8 pixelColorBuffer[HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
	memset(pixelColorBuffer, 0, HARDWARE_SPRITE_PIXEL_BUFFER_SIZE);

	// HARDWARE_SPRITE_TO_SCREEN_X_OFFSET accounts for
	// border and HBlank to match with screen background.
	vic->DrawBackgroundRowToBuffer(lineNumber, pixelColorBuffer + HARDWARE_SPRITE_TO_SCREEN_X_OFFSET);
				
	DrawBufferOnLine(
		0,
		screenLineNumber,
		pixelColorBuffer + HARDWARE_SPRITE_TO_SCREEN_X_OFFSET,
		SCREEN_WIDTH);

	// Draw sprites.
	// NOTE: Drawing all of them afterwards ignores the sprite
	// to background priority VIC setting.
	auto bus = CBus::GetInstance();
	u8 spriteEnable = bus->PeekDevice(eBusVic, 0xD015);
	if (spriteEnable != 0)
	{
		// Process sprites in reverse order to account for sprite priority.
		for (int spriteIndex = NUMBER_OF_HARDWARE_SPRITES - 1; spriteIndex > -1; --spriteIndex)
		{
			if (!vic->IsSpriteEnabled(spriteIndex))
			{
				continue;
			}

			if (vic->IsSpriteOnLine(spriteIndex, lineNumber))
			{
				u8 spriteYPosition = vic->GetSpriteYPosition(spriteIndex);
				u8 spriteRowOffset = (lineNumber - spriteYPosition) / vic->GetSpriteVerticalScale(spriteIndex);

				// 48 bytes in case of horizontal scaling.
				static u8 pixelColorBuffer[HARDWARE_SPRITE_WIDTH * 2];
				memset(pixelColorBuffer, 0, HARDWARE_SPRITE_WIDTH * 2);
				
				vic->DrawSpriteRowToBuffer(spriteIndex, spriteRowOffset, pixelColorBuffer);
				
				// Adjust X for border and HBlank to match with background.
				DrawBufferOnLine(
					vic->GetSpriteXPosition(spriteIndex) - HARDWARE_SPRITE_TO_SCREEN_X_OFFSET,
					screenLineNumber,
					pixelColorBuffer,
					vic->GetSpriteWidth(spriteIndex));
			}
		}
	}
}
