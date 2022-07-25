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

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

#define CHARACTER_WIDTH 8
#define CHARACTER_HEIGHT 8
#define NUMBER_OF_CHARACTERS 256

#define SCREEN_CHAR_WIDTH (SCREEN_WIDTH / CHARACTER_WIDTH)
#define SCREEN_CHAR_HEIGHT (SCREEN_HEIGHT / CHARACTER_HEIGHT)

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
void InitializeColors(void);
void SetColorValuesFromInt(SDL_Color* color, unsigned int value);
void Draw(void);
void ClearScreen(void);
void DrawRectangle(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char colorCode);
void InitializeCharacterSet(void);
void ShutdownCharacterSet(void);
void DrawCharacters(void);
void DrawCharacter(unsigned int x, unsigned int y, char shapeCode, char colorCode);
void PopulateCharacterSetSurfaceFromCharacterSet(SDL_Surface* characterSetSurface);
void PopulateCharacterSurfaceFromCharacter(SDL_Surface* characterSurface, unsigned int characterIndex);

void uiloop();
static int getch_lower_(int block);
std::vector<char> getKeystroke();
bool isKeystroke(std::vector<char>& keystroke, const std::vector<char>& k);

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
uint64_t total_cycles = 0;
char charbuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

static char CMOS6569TextMap[128] = 
{    '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
    'P','Q','R','S','T','U','V','W','X','Y','Z','[','x',']','^','x',
    ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
    '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
    'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
    'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
    ' ','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
    'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x' };

const std::vector<char> ESC = { 27 };
const std::vector<char> KEYUP = { 27, 91, 65 }; 
const std::vector<char> KEYDOWN = { 27, 91, 66 }; 
const std::vector<char> KEYLEFT = { 27, 91, 68 }; 
const std::vector<char> KEYRIGHT = { 27, 91, 67 }; 


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
            activescreen_ = new std::atomic<int>();
            *activescreen_ = 0;
            for (int i=0; i<2; i++) {
                screenbuffer_[i] = (uint8_t*)calloc(SCREEN_WIDTH * SCREEN_HEIGHT, sizeof(uint8_t));
            }
        }
        ~EMCScreen() {
            for (int i=0; i<2; i++) {
                free(screenbuffer_[i]);
            }        
            delete(activescreen_);
        }

        std::atomic<int> *activescreen_;
        uint8_t *screenbuffer_[2];
    public:
        void DrawPixels(u8* screenBuffer, VICRect* area) {
        }
        void DrawChar(u16 address, u8 c) {
        }
        int cnt_ = 0;
        void DrawChars(u8* memory) {
            memcpy(charbuffer, memory, SCREEN_WIDTH * SCREEN_HEIGHT);
        }
    public:
};


void runloop() {
    int target_cycles = 1023000; //NTSC
    int cycles_100_ms = target_cycles / 10;
    int cycles = 0;

    /*while (_run)*/ {
        cycles = 0;
        uint64_t ts = now();

        while (true) {
            cycles += cbm64->Cycle();
            if (cycles >= cycles_100_ms) {
                total_cycles += cycles;
                uint64_t elapsed = now() - ts;
                if (elapsed < 100) {
                    //usleep((100-elapsed) * 1000);
                }
                cycles = cycles - cycles_100_ms;
                break;
            }
        }

        /*
        printf("\e[?25l"); //cursor off
        uiloop();
        //usleep(100000);
        printf("\e[?25h"); //cursor on
        */
    }
}


void uiloop() {
    std::cout << "\033[0;0H" << std::endl; 
    std::cout << "\u001b[44m" << std::endl; 
    for (int y=0;y<25;y++) {
        for (int x=0;x<40;x++) {
            uint8_t m = charbuffer[y*40+x];
            char c = '_';
            if (m < 128) {
                c = CMOS6569TextMap[m]; 
            }
            std::cout << c;
        }
        std::cout << std::endl;
    }
    std::cout << "\u001b[0m" << std::endl;

    if (emcScreen_ != NULL)
    {
        int backgroundColor = emcScreen_->GetBackgroundColor();
        std::cout << "Background Color: " << backgroundColor << std::endl;
    }
}


//https://stackoverflow.com/questions/36428098/c-how-to-check-if-my-input-bufferstdin-is-empty
static int getch_lower_(int block)
{
    struct termios tc = {};
    int status;
    char rdbuf;
    // retrieve initial settings.
    if (tcgetattr(STDIN_FILENO, &tc) < 0)
        perror("tcgetattr()");
    // non-canonical mode; no echo.
    tc.c_lflag &= ~(ICANON | ECHO);
    tc.c_cc[VMIN] = block ? 1 : 0; // bytes until read unblocks.
    tc.c_cc[VTIME] = 0; // timeout.
    if (tcsetattr(STDIN_FILENO, TCSANOW, &tc) < 0)
        perror("tcsetattr()");
    // read char.
    if ((status = read(STDIN_FILENO, &rdbuf, 1)) < 0)
        perror("read()");
    // restore initial settings.
    tc.c_lflag |= (ICANON | ECHO);
    if (tcsetattr(STDIN_FILENO, TCSADRAIN, &tc) < 0)
        perror("tcsetattr()");
    return (status > 0) ? rdbuf : EOF;
}


std::vector<char> getKeystroke() {
    std::vector<char> keystroke;
    int cc = getch_lower_(1);
    keystroke.push_back(cc);
    while (cc != -1) {
        cc = getch_lower_(0);
        if (cc != -1) {
            keystroke.push_back(cc);
        }
    }
    return keystroke;
}


bool isKeystroke(std::vector<char>& keystroke, const std::vector<char>& k) {
    if (k.size() != keystroke.size()) {
        return false;
    }
    for (int x=0; x<keystroke.size(); x++) {
        if (keystroke[x] != k[x]) {
            return false;
        }
    }
    return true;
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

    InitializeCharacterSet();

    //Set the hires time callbacks
    HiresTimeImpl hiresTime;
    cbm64->SetHiresTimeProvider(&hiresTime);

    //Subscribe Host hardware related VIC Code
    emcScreen_ = new EMCScreen();
    CMOS6569* vic = cbm64->GetVic();
    unsigned char* screenBuffer = vic->RegisterHWScreen(emcScreen_);

    std::cout << "\033c" << std::endl;

    MainLoop();

    std::cout << "press a key to end..." << std::endl;
    std::cout << "ended..." << std::endl;

    ShutdownCharacterSet();
    SDL_DestroyWindow(Window);
	SDL_DestroyRenderer(Renderer);
    SDL_Quit();
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

	Draw();

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
				//KeyCodeStates[event->key.keysym.scancode] = 1;

                std::vector<char> keystroke;// = getKeystroke();
                //https://sta.c64.org/cbm64pet.html
                cout << "Typed: " << char(event->key.keysym.sym) << "(" << int(event->key.keysym.sym) << ")" << endl;
                keystroke.push_back(event->key.keysym.sym);
                
                char c = keystroke[0];
                if (c >= 'a' && c <= 'z') {
                    c += 'A' - 'a';
                } else if (c == 10) {
                    c = 13;
                } else if (isKeystroke(keystroke, ESC)) {
                    c = 3;
                } else if (isKeystroke(keystroke, KEYUP)) {
                    c = 145;
                } else if (isKeystroke(keystroke, KEYDOWN)) {
                    c = 17;
                } else if (isKeystroke(keystroke, KEYLEFT)) {
                    c = 157;
                } else if (isKeystroke(keystroke, KEYRIGHT)) {
                    c = 29;
                } else if (c == 127) {
                    c = 20;
                }
                cbm64->GetCia1()->AddKeyStroke(c);
			}

			break;
		}

		case SDL_KEYUP:
		{
			if (event->key.repeat == 0)
			{
				//KeyCodeStates[event->key.keysym.scancode] = 0;
			}

			break;
		}
	}
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

SDL_Surface* CreateSurface(unsigned int width, unsigned int height)
{
	Uint32 rmask, gmask, bmask, amask;

	// SDL interprets each pixel as a 32-bit number,
	// so our masks must depend on the endianness (byte order) of the machine.
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	rmask = 0xff000000;
	gmask = 0x00ff0000;
	bmask = 0x0000ff00;
	amask = 0x000000ff;
#else
	rmask = 0x000000ff;
	gmask = 0x0000ff00;
	bmask = 0x00ff0000;
	amask = 0xff000000;
#endif

	return SDL_CreateRGBSurface
		(
			0,
			width, height,
			32,
			rmask, gmask, bmask, amask
		);
}

void Draw(void)
{
	ClearScreen();

	DrawCharacters();
	//DrawSprites();

	SDL_RenderPresent(Renderer);
}

void ClearScreen(void)
{
	SDL_Color* backgroundColor = &Colors[emcScreen_->GetBackgroundColor()];

	SDL_SetRenderDrawColor(Renderer, backgroundColor->r, backgroundColor->g, backgroundColor->b, 0xff);
	SDL_RenderClear(Renderer);
}

void DrawRectangle(unsigned int x, unsigned int y, unsigned int width, unsigned int height, char colorCode)
{
	SDL_Color* color = &Colors[colorCode];

	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = width;
	rect.h = height;

	SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, color->a);
	
	SDL_RenderFillRect(Renderer, &rect);
}

void InitializeCharacterSet(void)
{
	SDL_Surface* characterSetSurface = CreateSurface(CHARACTER_WIDTH, NUMBER_OF_CHARACTERS * CHARACTER_HEIGHT);
	if (characterSetSurface == NULL)
	{
		// TODO: Generate error.
		return;
	}

	PopulateCharacterSetSurfaceFromCharacterSet(characterSetSurface);

	CharacterSetTexture = SDL_CreateTextureFromSurface(Renderer, characterSetSurface);

	SDL_FreeSurface(characterSetSurface);
	characterSetSurface = NULL;
}

void ShutdownCharacterSet(void)
{
	SDL_DestroyTexture(CharacterSetTexture);
}

void DrawCharacters(void)
{
	char shapeCode;
	char colorCode;

	unsigned int columnIndex;
	unsigned int rowIndex;

	for (rowIndex = 0; rowIndex < SCREEN_CHAR_HEIGHT; ++rowIndex)
	{
		for (columnIndex = 0; columnIndex < SCREEN_CHAR_WIDTH; ++columnIndex)
		{
            shapeCode = charbuffer[(rowIndex * SCREEN_CHAR_WIDTH) + columnIndex];
			if (shapeCode != 32)
			{
				colorCode = COLOR_LIGHT_BLUE; // TODO: Pull from color RAM?
				DrawCharacter(columnIndex * CHARACTER_WIDTH, rowIndex * CHARACTER_HEIGHT, shapeCode, colorCode);
			}
		}
	}
}

void PopulateCharacterSetSurfaceFromCharacterSet(SDL_Surface* characterSetSurface)
{
	unsigned int characterIndex;

	SDL_Rect characterDestinationRect;
	
	SDL_Surface* characterSurface = CreateSurface(CHARACTER_WIDTH, CHARACTER_HEIGHT);
	if (characterSurface == NULL)
	{
		// TODO: Generate error.
		return;
	}

	characterDestinationRect.w = CHARACTER_WIDTH;
	characterDestinationRect.h = CHARACTER_HEIGHT;

	// Initial destination location for the first character graphic.
	characterDestinationRect.x = 0;
	characterDestinationRect.y = 0;

	for (characterIndex = 0; characterIndex < NUMBER_OF_CHARACTERS; ++characterIndex)
	{
		PopulateCharacterSurfaceFromCharacter(characterSurface, characterIndex);

		// Copy it over.
		SDL_BlitSurface
			(
				characterSurface,
				NULL,
				characterSetSurface,
				&characterDestinationRect
			);
		
		// Determine destination location for next character graphic.
		characterDestinationRect.y += CHARACTER_HEIGHT;
	}

	SDL_FreeSurface(characterSurface);
	characterSurface = NULL;
}

void PopulateCharacterSurfaceFromCharacter(SDL_Surface* characterSurface, unsigned int characterIndex)
{
	unsigned int x, y;
	char colorCode;
	unsigned int* pixels;

	if (SDL_MUSTLOCK(characterSurface))
	{
		SDL_LockSurface(characterSurface);
	}

	pixels = (unsigned int*)characterSurface->pixels;

	for (y = 0; y < CHARACTER_HEIGHT; ++y)
	{
		for (x = 0; x < CHARACTER_WIDTH; ++x)
		{
            int charRomCharOffset = characterIndex * CHARACTER_HEIGHT;
            char charRowByte = cbm64->GetCharRom()->Peek(CHARROMSTART + charRomCharOffset + y);
            colorCode = charRowByte & (1 << (CHARACTER_WIDTH - x - 1));

			// TODO: Actually, check color code.
			*((unsigned int*)pixels) = colorCode != 0 ?
				SDL_MapRGBA(characterSurface->format, 255, 255, 255, 255) :
				SDL_MapRGBA(characterSurface->format, 0, 0, 0, 0);
			
			++pixels;
		}
	}

	if (SDL_MUSTLOCK(characterSurface))
	{
		SDL_UnlockSurface(characterSurface);
	}
}

void DrawCharacter(unsigned int x, unsigned int y, char shapeCode, char colorCode)
{
	SDL_Color* color = &Colors[colorCode];

	SDL_Rect sourceRect;
	SDL_Rect destinationRect;

	sourceRect.x = 0;
	sourceRect.y = shapeCode * CHARACTER_HEIGHT;
	sourceRect.w = CHARACTER_WIDTH;
	sourceRect.h = CHARACTER_HEIGHT;

	destinationRect.x = x;
	destinationRect.y = y;
	destinationRect.w = CHARACTER_WIDTH;
	destinationRect.h = CHARACTER_HEIGHT;

	SDL_SetTextureColorMod(CharacterSetTexture, color->r, color->g, color->b);
	SDL_RenderCopy(Renderer, CharacterSetTexture, &sourceRect, &destinationRect);
}
