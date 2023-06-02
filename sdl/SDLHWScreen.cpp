/*
 *  SDLHWScreen.cpp
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

#include "SDLHWScreen.h"


SDL_Color Colors[NUMBER_OF_COLORS];
SDL_Texture* CharacterSetTexture;


void SDLHWScreen::Init(){
	SDL_InitSubSystem(SDL_INIT_VIDEO);

	RenderScale.x = 3;
	RenderScale.y = 3;

	Window = SDL_CreateWindow(
		"6510",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH * RenderScale.x, SCREEN_HEIGHT * RenderScale.y,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
	);

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_RenderSetScale(Renderer, (float)RenderScale.x, (float)RenderScale.y);
	SDL_RenderSetLogicalSize(Renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

	InitializeColors();
}


void SDLHWScreen::Shutdown(){
	SDL_DestroyWindow(Window);
	SDL_DestroyRenderer(Renderer);

	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


void SDLHWScreen::OnRasterLineCompleted(unsigned int lineNumber){
	// TODO: Catch & translate border lines & V blank lines.
	DrawScreenLine(lineNumber);

	if (lineNumber == fieldLineHeight){
		SDL_RenderPresent(Renderer);
		
		// Clear render display (avoids artifacts in "border" in SDL window when resized).
		SDL_Color* color = &Colors[COLOR_BLACK];

		SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, 0xff);
		SDL_RenderClear(Renderer);
	}
}


void SDLHWScreen::DrawChar(u16 address, u8 c){

}


void SDLHWScreen::DrawChars(u8* memory){

}


void SDLHWScreen::InitializeColors(void){
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


void SDLHWScreen::SetColorValuesFromInt(SDL_Color* color, unsigned int value){
	color->r = (value >> 16) & 0xff;
	color->g = (value >> 8) & 0xff;
	color->b = value & 0xff;
	color->a = 0xff;
}


void SDLHWScreen::DrawBufferOnLine(u16 screenXPosition, u8 screenYPosition, u8* pixelColorBuffer, unsigned int numberOfPixels){
	// Draw to renderer from pixel buffers.
	SDL_Rect rect;
	rect.x = screenXPosition;
	rect.y = screenYPosition;
	rect.w = 1;
	rect.h = 1;

	for (unsigned int x = 0; x < numberOfPixels; ++x, ++rect.x){
		if (pixelColorBuffer[x] == 0){
			continue;
		}

		// Minus 1 to account for using 0 for transparency.
		u8 colorCode = pixelColorBuffer[x] - 1;
		SDL_Color* color = &Colors[colorCode & 0x0F];
		SDL_SetRenderDrawColor(Renderer, color->r, color->g, color->b, color->a);
		SDL_RenderFillRect(Renderer, &rect);
	}
}


void SDLHWScreen::DrawScreenLine(unsigned int lineNumber){
	int screenLineNumber = lineNumber - HARDWARE_SPRITE_TO_SCREEN_Y_OFFSET;
	if (screenLineNumber < 0){
		return;
	}

	if (screenLineNumber >= SCREEN_HEIGHT){
		return;
	}

	// Draw background.
	// TODO: Does not account for background color in extended color mode.
	SDL_Rect rect;
	rect.y = screenLineNumber;
	rect.h = 1;

	rect.x = 0;
	rect.w = SCREEN_WIDTH;

	SDL_Color* backgroundColor = &Colors[vic->Peek(0xD021) % 16];
	SDL_SetRenderDrawColor(Renderer, backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);
	SDL_RenderFillRect(Renderer, &rect);

	// Graphics over background color.
	static u8 pixelColorBuffer[HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
	memset(pixelColorBuffer, 0, sizeof(pixelColorBuffer));

	vic->RenderGraphicsToBuffer(pixelColorBuffer);
	
	// HARDWARE_SPRITE_TO_SCREEN_X_OFFSET accounts for
	// border and HBlank to match with screen background.
	DrawBufferOnLine(
		0,
		screenLineNumber,
		pixelColorBuffer + HARDWARE_SPRITE_TO_SCREEN_X_OFFSET,
		SCREEN_WIDTH);
}
