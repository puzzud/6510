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


class CVICHWScreen{
public:
		u8 GetBorderColor(){return mBorderColor;};
		void SetBorderColor(u8 color){mBorderColor = color;};
		u8 GetBackgroundColor(){return mBackGroundColor;};
		void SetBackgroundColor(u8 color){mBackGroundColor = color;};

		virtual void DrawChar(u16 address, u8 c)  = 0;
		virtual void DrawChars(u8* memory) = 0;
protected:
		u8 mBorderColor;
		u8 mBackGroundColor;
};


class CMOS6569 : public CDevice{
	private:
		CVICHWScreen* mRenderer;
		u8 chipMemoryControlRegister;
		u8 mColorRam[0xDBFF-0xD800];
		CBus* mBus;	
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
