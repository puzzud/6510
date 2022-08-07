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

#define NTSC_FIELD_CYCLES_PER_LINE           65
#define NTSC_FIELD_LINE_WIDTH                520 /* NTSC_FIELD_CYCLES_PER_LINE * PIXELS_PER_CYCLE */
#define NTSC_FIELD_LINE_HEIGHT               263

#define PAL_FIELD_CYCLES_PER_LINE            63
#define PAL_FIELD_LINE_WIDTH                 504 /* PAL_FIELD_CYCLES_PER_LINE * PIXELS_PER_CYCLE */
#define PAL_FIELD_LINE_HEIGHT                312

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
#define SCREEN_CYCLES_PER_LINE               SCREEN_CHAR_WIDTH

#define NUMBER_OF_HARDWARE_SPRITES           8
#define HARDWARE_SPRITE_WIDTH                24
#define HARDWARE_SPRITE_HEIGHT               21
#define HARDWARE_SPRITE_X_POSITION_MAX       511
#define HARDWARE_SPRITE_PIXEL_BUFFER_SIZE    (HARDWARE_SPRITE_X_POSITION_MAX + (HARDWARE_SPRITE_WIDTH * 2))
#define HARDWARE_SPRITE_BYTE_COUNT           63 /* 3 bytes per row. 21 rows */
#define HARDWARE_SPRITE_DATA_BLOCK_SIZE      (HARDWARE_SPRITE_BYTE_COUNT + 1) /* 64 */
#define HARDWARE_SPRITE_TO_SCREEN_X_OFFSET   24
#define HARDWARE_SPRITE_TO_SCREEN_Y_OFFSET   50


typedef enum _eByteRenderMode{
	eByteRenderModeCharacter,
	eByteRenderModeSprite
}eByteRenderMode;


class CVICHWScreen{
public:
		virtual void OnRasterLineCompleted(unsigned int lineNumber){};
		virtual void DrawChar(u16 address, u8 c)  = 0;
		virtual void DrawChars(u8* memory) = 0;
protected:
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
		// TODO: Also maybe used in future to actually render backgrounds
		// (with proper sprite priority).
		// Note that each line is the same length as
		// spriteFieldLinePixelColorBuffers.
		// Note that background colors (especially in case of extended color mode),
		// are not rendered to preserve transparency
		// for collision purposes.
		u8 backgroundFieldLinePixelColorBuffer[HARDWARE_SPRITE_PIXEL_BUFFER_SIZE];
	protected:
	public:
		CMOS6569();
		virtual ~CMOS6569(){}
		u8 GetDeviceID();

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
		unsigned int GetSpriteHorizontalScale(unsigned int spriteIndex);
		unsigned int GetSpriteVerticalScale(unsigned int spriteIndex);
		u8 GetSpriteWidth(unsigned int spriteIndex);
		u8 GetSpriteHeight(unsigned int spriteIndex);
		bool IsSpriteOnLine(unsigned int spriteIndex, unsigned int lineNumber);
		u8 GetSpritePointerValue(unsigned int spriteIndex);
		u16 GetSpriteDataMemoryOffset(unsigned int spriteIndex);
		u8 GetSpriteColor(unsigned int spriteIndex);
		bool IsSpriteMultiColor(unsigned int spriteIndex);
		void TestSpriteCollision();

		void SetChar(u16 address, u8 c); //temp
		
		//Hardware dependend calls
		void RegisterHWScreen(CVICHWScreen* screen);
		void HWNeedsRedraw();

		void DrawByteToBuffer(u8 byte, u8* pixelColorBuffer, u8* colorCodes, eByteRenderMode mode, bool multiColor, unsigned int horizontalScale = 1);
		void DrawBackgroundRowToBuffer(unsigned int fieldLineNumber, u8* pixelColorBuffer);
		void DrawSpriteRowToBuffer(unsigned int spriteIndex, unsigned int rowIndex, u8* pixelColorBuffer);
};




#endif //MOS6569_H
