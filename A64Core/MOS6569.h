/*
 *  MOS6569.h 
 *  A64Emulator
 *  
 *  6569 (PAL) Video Chip (VIC II) Emulator  
 *
 *  Created by Bruno Keymolen on 06/07/08.
 *  Copyright 2008 Bruno Keymolen. All rights reserved.
 *
 */

#ifndef MOS6569_H
#define MOS6569_H

#include "Common.h"
#include "Device.h"
#include "Bus.h"

#define CHARACTER_MEMORY_BANK_SIZE           0x0800
#define SCREEN_MEMORY_BANK_SIZE              0x0400

#define SCREEN_WIDTH                         320
#define SCREEN_HEIGHT                        200

#define CHARACTER_WIDTH                      8
#define CHARACTER_HEIGHT                     8
#define NUMBER_OF_CHARACTERS                 256

#define SCREEN_CHAR_WIDTH                    (SCREEN_WIDTH / CHARACTER_WIDTH)
#define SCREEN_CHAR_HEIGHT                   (SCREEN_HEIGHT / CHARACTER_HEIGHT)

#define PIXELS_PER_CYCLE                     8

#define NTSC_FIELD_LINE_WIDTH                520
#define NTSC_FIELD_LINE_HEIGHT               263
//#define NTSC_FIELD_CYCLES_PER_LINE         NTSC_FIELD_LINE_WIDTH / PIXELS_PER_CYCLE /* 65 */

#define PAL_FIELD_LINE_WIDTH                 504
#define PAL_FIELD_LINE_HEIGHT                312
//#define PAL_FIELD_CYCLES_PER_LINE          PAL_FIELD_LINE_WIDTH / PIXELS_PER_CYCLE /* 63 */

#define NUMBER_OF_HARDWARE_SPRITES           8
#define HARDWARE_SPRITE_WIDTH                24
#define HARDWARE_SPRITE_HEIGHT               21
#define HARDWARE_SPRITE_X_POSITION_MAX       511
#define HARDWARE_SPRITE_PIXEL_BUFFER_SIZE    (HARDWARE_SPRITE_X_POSITION_MAX + (HARDWARE_SPRITE_WIDTH * 2))
#define HARDWARE_SPRITE_BYTE_COUNT           63 /* 3 bytes per row. 21 rows */
#define HARDWARE_SPRITE_DATA_BLOCK_SIZE      (HARDWARE_SPRITE_BYTE_COUNT + 1) /* 64 */
#define HARDWARE_SPRITE_TO_SCREEN_X_OFFSET   24
#define HARDWARE_SPRITE_TO_SCREEN_Y_OFFSET   50


typedef enum
{
	NTSC,
	PAL
} VideoFormatStandard;


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


typedef enum _eByteRenderMode{
	eByteRenderModeCharacter,
	eByteRenderModeCharacterCollision,
	eByteRenderModeSprite
}eByteRenderMode;

class CMOS6569;

class CVICHWScreen{
public:
		CVICHWScreen();
		void SetVic(CMOS6569* vic);
		void SetVideoFormatStandard(VideoFormatStandard videoFormatStandard);

		VideoFormatStandard GetVideoFormatStandard();
		u16 GetFieldLineWidth();
		u16 GetFieldLineHeight();

		unsigned int AdvanceRasterLine();

		virtual void OnRasterLineCompleted(unsigned int lineNumber){};
		virtual void DrawChar(u16 address, u8 c)  = 0;
		virtual void DrawChars(u8* memory) = 0;
protected:
		VideoFormatStandard videoFormatStandard;
		u16 fieldLineWidth;
		u16 fieldLineHeight;
		unsigned int rasterLine;

		CMOS6569* vic;
};


class CMOS6569 : public CDevice{
	private:
		u8 mRegs[47];
		u8& chipMemoryControlRegister = *(mRegs + (0xD018 - 0xD000));
		u8 mColorRam[0xDBFF-0xD800];

		bool irq;

		u16 rasterLine;
		u8 perLineClockCycle;

		u16 rasterLineCompare = 0;

		VideoFormatStandard videoFormatStandard;
		u8 fieldCyclesPerLine;
		unsigned int cyclesPerFrame;

		CBus* mBus;
		CVICHWScreen* mRenderer;

		// Used for determining sprite to sprite collisions.
		// TODO: Maybe used in future to actually render sprites.
		// Each sprite gets a dedicated line which is then
		// used to compare between each sprite.
		// Note that each line is long enough to contain
		// double width sprites at their max X position,
		// done so to prevent overflow and bound checking.
		u8 spriteFieldLinePixelColorBuffers[NUMBER_OF_HARDWARE_SPRITES][HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
		
		// Used for determine sprite to background collisions.
		// Note that each line is the same length as
		// spriteFieldLinePixelColorBuffers.
		// Note that background colors (especially in case of extended color mode),
		// are not rendered to preserve transparency for collision purposes.
		// Note that multicolor bit pair 01 is represented with transparency,
		// to facilitate how collision detection, meaning this buffer
		// cannot be used directly for rendering.
		u8 backgroundFieldLinePixelColorBuffer[HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
	protected:
		void RenderFieldLinePixelColorBuffers();
		void TestSpriteCollision();
	public:
		CMOS6569();
		virtual ~CMOS6569(){}
		u8 GetDeviceID();

		VideoFormatStandard GetVideoFormatStandard();
		void SetVideoFormatStandard(VideoFormatStandard videoFormatStandard);
		u16 GetCyclesPerFrame();
		void UpdateCyclesPerFrame();

        void Cycle();

		u8 Peek(u16 address);
		int Poke(u16 address, u8 val); 

		bool GetIRQ();

		u16 GetCharacterMemoryOffset();
		u16 GetScreenMemoryOffset();
		u16 GetSpritePointersMemoryOffset();

		// Sprite
		bool IsSpriteEnabled(unsigned int spriteIndex);
		u16 GetSpriteXPosition(unsigned int spriteIndex);
		u8 GetSpriteYPosition(unsigned int spriteIndex);
		void SetSpriteXPosition(unsigned int spriteIndex, u16 xPosition);
		void SetSpriteYPosition(unsigned int spriteIndex, u8 yPosition);
		unsigned int GetSpriteHorizontalScale(unsigned int spriteIndex);
		unsigned int GetSpriteVerticalScale(unsigned int spriteIndex);
		u8 GetSpriteWidth(unsigned int spriteIndex);
		u8 GetSpriteHeight(unsigned int spriteIndex);
		bool IsSpriteOnLine(unsigned int spriteIndex, unsigned int lineNumber);
		u8 GetSpritePointerValue(unsigned int spriteIndex);
		u16 GetSpriteDataMemoryOffset(unsigned int spriteIndex);
		u8 GetSpriteColor(unsigned int spriteIndex);
		bool IsSpriteMultiColor(unsigned int spriteIndex);

		void SetChar(u16 address, u8 c); //temp
		
		//Hardware dependend calls
		void RegisterHWScreen(CVICHWScreen* screen);
		void HWNeedsRedraw();

		void DrawByteToBuffer(u8 byte, u8* pixelColorBuffer, u8* colorCodes, eByteRenderMode mode, bool multiColor, unsigned int horizontalScale = 1);
		void DrawBackgroundRowToBuffer(unsigned int fieldLineNumber, eByteRenderMode mode, u8* pixelColorBuffer);
		void DrawSpriteRowToBuffer(unsigned int spriteIndex, unsigned int rowIndex, u8* pixelColorBuffer);

		// This method takes all the background & sprite field line pixel color
		// buffers and renders them together, taking into account sprite priority
		// over other sprites and background graphics. This method should not be
		// called internally, as it is purely graphical.
		void RenderGraphicsToBuffer(u8* pixelColorBuffer);
};




#endif //MOS6569_H
