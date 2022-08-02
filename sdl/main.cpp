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
void DrawScreenLine(unsigned int lineNumber);

class HiresTimeImpl;
class EMCScreen;

CBM64Main* cbm64 = NULL;
EMCScreen* emcScreen_ = NULL;

SDL_Window* Window;
SDL_Renderer* Renderer;
SDL_Point RenderScale;
SDL_Color Colors[NUMBER_OF_COLORS];
SDL_Texture* CharacterSetTexture;

bool _run = true;
uint64_t remainingCycles = 0;

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

	if (argc > 1)
	{
		cbm64->LoadAppWithoutBasic(argv[1]);
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
}

void OnInputTextInputEvent(SDL_Event* event)
{
    char keySymbol = event->text.text[0];
    //cout << "Text Input: " << char(keySymbol) << endl;

    char keyStroke = keySymbol;

    if (keySymbol >= 'a' && keySymbol <= 'z')
    {
        keyStroke = keySymbol + 'A' - 'a';
    }

    cbm64->GetCia1()->AddKeyStroke(char(keyStroke));
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

void DrawScreenLine(unsigned int lineNumber)
{
	// TODO: Figure out this number!
#define SCREEN_START_FIELD_LINE 51
	int screenLineNumber = lineNumber - SCREEN_START_FIELD_LINE;
	if (screenLineNumber < 0)
	{
		return;
	}

	if (screenLineNumber >= SCREEN_HEIGHT)
	{
		return;
	}

	auto bus = CBus::GetInstance();
	auto vic = cbm64->GetVic();
	u16 vicMemoryBankStartAddress = bus->GetVicMemoryBankStartAddress();
	u16 vicScreenMemoryStartAddress = vicMemoryBankStartAddress + vic->GetScreenMemoryOffset();
	u16 vicCharacterMemoryStartAddress = vicMemoryBankStartAddress + vic->GetCharacterMemoryOffset();

	SDL_Rect rect;
	rect.y = screenLineNumber;
	rect.h = 1;

	rect.x = 0;
	rect.w = SCREEN_WIDTH;

	SDL_Color* backgroundColor = &Colors[emcScreen_->GetBackgroundColor() % 16];
	SDL_SetRenderDrawColor(Renderer, backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
	SDL_RenderFillRect(Renderer, &rect);

	rect.w = 0;

	unsigned int rowIndex = screenLineNumber / CHARACTER_HEIGHT;
	unsigned int characterRowIndex = screenLineNumber % CHARACTER_HEIGHT;

	// 1 column (1 byte) per cycle.
	for (unsigned int columnIndex = 0; columnIndex < SCREEN_CHAR_WIDTH; ++columnIndex)
	{
		int characterScreenMemoryOffset = (rowIndex * SCREEN_CHAR_WIDTH) + columnIndex;

		unsigned char colorCode = bus->PeekDevice(eBusVic, 0xD800 + characterScreenMemoryOffset) & 0x0F;
		unsigned char shapeCode = bus->PeekDevice(eBusRam, vicScreenMemoryStartAddress + characterScreenMemoryOffset);

		int characterRomCharOffset = shapeCode * CHARACTER_HEIGHT;
		
		bus->SetMode(eBusModeVic);
		unsigned char charRowByte = bus->Peek(vicCharacterMemoryStartAddress + characterRomCharOffset + characterRowIndex);

		rect.w = 1;
		for (int x = 0; x < CHARACTER_WIDTH; ++x)
		{
            if (charRowByte & (1 << (CHARACTER_WIDTH - x - 1)))
			{
				rect.x = (columnIndex * CHARACTER_WIDTH) + x;

				SDL_Color* color = &Colors[colorCode % 16];
				SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, color->a);
				SDL_RenderFillRect(Renderer, &rect);
			}
		}
	}

	// Draw horizontal slices of sprite rectangles (for now).
	// NOTE: Drawing all of them afterwards ignores the sprite
	// to background priority VIC setting.
	u8 spriteEnable = bus->PeekDevice(eBusVic, 0xD015);
	if (spriteEnable != 0)
	{
		u8 spriteXPositionMsbs = bus->PeekDevice(eBusVic, 0xD010);
		u8 spriteXExpands = bus->PeekDevice(eBusVic, 0xD01D);
		u8 spriteYExpands = bus->PeekDevice(eBusVic, 0xD017);

		// Process sprites in reverse order to account for sprite priority.
		for (int spriteIndex = 8 - 1; spriteIndex > -1; --spriteIndex)
		{
			if ((spriteEnable & (1 << spriteIndex)) == 0)
			{
				continue;
			}

			u8 spriteYPosition = bus->PeekDevice(eBusVic, 0xD001 + (spriteIndex * 2));
			unsigned int spriteHeight = 21;
			if ((spriteYExpands & (1 << spriteIndex)) != 0)
			{
				spriteHeight *= 2;
			}

			if (lineNumber >= spriteYPosition && lineNumber <= (spriteYPosition + spriteHeight))
			{
				u8 spriteXPositionLsb = bus->PeekDevice(eBusVic, 0xD000 + (spriteIndex * 2));

				int spriteXPosition = spriteXPositionLsb;
				if ((spriteXPositionMsbs & (1 << spriteIndex)) != 0)
				{
					spriteXPosition += 0x100;
				}

				spriteXPosition -= 23; // Adjust for border and HBlank.

				rect.x = spriteXPosition;
				rect.w = 24;
				if ((spriteXExpands & (1 << spriteIndex)) != 0)
				{
					rect.w *= 2;
				}

				u8 colorCode = bus->PeekDevice(eBusVic, 0xD027 + spriteIndex);
				SDL_Color* color = &Colors[colorCode % 16];
				SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, color->a);
				SDL_RenderFillRect(Renderer, &rect);
			}
		}
	}
}
