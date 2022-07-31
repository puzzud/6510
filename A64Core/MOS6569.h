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
#define SCREEN_CYCLES_PER_LINE               SCREEN_CHAR_WIDTH

#define NTSC_FIELD_CYCLES_PER_LINE           65
#define NTSC_FIELD_LINE_WIDTH                520 // NTSC_FIELD_CYCLES_PER_LINE * PIXELS_PER_CYCLE
#define NTSC_FIELD_LINE_HEIGHT               263

#define PAL_FIELD_CYCLES_PER_LINE            63
#define PAL_FIELD_LINE_WIDTH                 504 // PAL_FIELD_CYCLES_PER_LINE * PIXELS_PER_CYCLE
#define PAL_FIELD_LINE_HEIGHT                312


class CVICHWScreen{
public:
		u8 GetBorderColor(){return mBorderColor;};
		void SetBorderColor(u8 color){mBorderColor = color;};
		u8 GetBackgroundColor(){return mBackGroundColor;};
		void SetBackgroundColor(u8 color){mBackGroundColor = color;};

		virtual void OnRasterLineCompleted(unsigned int lineNumber){};
		virtual void DrawChar(u16 address, u8 c)  = 0;
		virtual void DrawChars(u8* memory) = 0;
protected:
		u8 mBorderColor;
		u8 mBackGroundColor;
};


class CMOS6569 : public CDevice{
	private:
		u8 mRegs[47];
		u8& chipMemoryControlRegister = *(mRegs + (0xD018 - 0xD000));
		u8 mColorRam[0xDBFF-0xD800];

		u16 rasterLine;
		u8 perLineClockCycle;

		CBus* mBus;
		CVICHWScreen* mRenderer;
	protected:
	public:
		CMOS6569();
		u8 GetDeviceID();

        void Cycle();

		u8 Peek(u16 address);
		int Poke(u16 address, u8 val); 

		u16 GetCharacterMemoryOffset();
		u16 GetScreenMemoryOffset();
		u16 GetSpritePointersMemoryOffset();

		void SetChar(u16 address, u8 c); //temp
		
		//Hardware dependend calls
		void RegisterHWScreen(CVICHWScreen* screen);
		void HWNeedsRedraw();
};




#endif //MOS6569_H
