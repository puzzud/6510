/*
 *  MOS6569.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 06/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MOS6569.h"
#include "Util.h"



static char CMOS6569TextMap[128] = 
				 {    '@','A','B','C','D','E','F','G','H','I','J','K','L','M','N','O',
					  'P','Q','R','S','T','U','V','W','X','Y','Z','[','x',']','^','x',
                      ' ','!','"','#','$','%','&','\'','(',')','*','+',',','-','.','/',
                      '0','1','2','3','4','5','6','7','8','9',':',';','<','=','>','?',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      ' ','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x',
                      'x','x','x','x','x','x','x','x','x','x','x','x','x','x','x','x' };


CMOS6569::CMOS6569(){
	memset(mRegs, 0, 47);
	memset(mColorRam, 0, (0xDBFF-0xD800));

	irq = false;

	rasterLine = 0;
	perLineClockCycle = 0;

	rasterLineCompare = 0;

	mBus = CBus::GetInstance(); 
	mBus->Register(eBusVic, this, 0xD000, 0xD3FF);
}

void CMOS6569::SetChar(u16 address, u8 c){
	cout << "???????????????" << CMOS6569TextMap[c] << flush;// << endl;
}

u8 CMOS6569::GetDeviceID(){
	return eBusVic;
}

void CMOS6569::Cycle(){
	if (++perLineClockCycle == NTSC_FIELD_CYCLES_PER_LINE){
		perLineClockCycle = 0;

		if (mRenderer != NULL){
			mRenderer->OnRasterLineCompleted((unsigned int)rasterLine);
		}

		if (++rasterLine == NTSC_FIELD_LINE_HEIGHT + 1){
			rasterLine = 0;
		}

		// TODO: Should this be called here or at a different point in time?
		if (rasterLine == rasterLineCompare){
			mRegs[0xD019 - 0xD000] |= 0x81; // Mark possible interrupt flags.
		}
	}

	bool rasterLineCompareIrqEnabled = (mRegs[0xD01A - 0xD000] & 0x01) != 0;
	bool rasterLineCompareMatched = (mRegs[0xD019 - 0xD000] & 0x01) != 0;
	irq = rasterLineCompareIrqEnabled && rasterLineCompareMatched;
}

u8 CMOS6569::Peek(u16 address){
	if(address >= 0xD800 && address <= 0xDBFF){
		// NOTE: On a real system the high nibble
		// of each byte is unreliable and essentially
		// random noise from recent bus operations.
		// Random data could be inserted on peek,
		// but for now it is AND'd out.
		return mColorRam[address-0xD800] & 0x0F;
	}

	// TODO: Implement mirroring.
	if(address >= 0xD000 && address <= (0xD000 + 47 - 1)){
		if(address == 0xD011){
			u8 val = mRegs[address-0xD000];

			// Bit 7 is different when read,
			// and is based on current position
			// of raster line;
			val &= 0x7F;
			val |= (rasterLine >> 1) & 0x80;
			return val;
		}else if (address == 0xD012){
			return (u8)(rasterLine & 0xFF);
		}

		return mRegs[address - 0xD000];
	}
	
	return 0xff;
}


int CMOS6569::Poke(u16 address, u8 val){
	if (address >= 0xD800 && address <= 0xDBFF){
		mColorRam[address - 0xD800] = val;
	}else if(address >= 0xD000 && address <= (0xD000 + 47 - 1)){
		// TODO: Implement mirroring.

		if (address == 0xD011){
			rasterLineCompare = (rasterLineCompare & 0xFF) | ((val & 0x80) << 1);
		}else if (address == 0xD012){
			rasterLineCompare = (rasterLineCompare & 0x100) | val;
		}else if (address == 0xD019){
			u8 previousVal = mRegs[address - 0xD000];
			val = previousVal & ~(val & 0x0F);
			val &= 0x7F;
			if ((val & 0x0F) != 0){
				val |= 0x80;
			}
		}else if (address == 0xD020){
			mRenderer->SetBorderColor(val);
		}else if (address == 0xD021){
			mRenderer->SetBackgroundColor(val);
		}

		mRegs[address - 0xD000] = val;
	}

	return 0;
}


bool CMOS6569::GetIRQ(){
	return irq;
}


u16 CMOS6569::GetCharacterMemoryOffset(){
	u8 vicCharacterMemoryBankIndex = (chipMemoryControlRegister >> 1) & 0x07;
	return vicCharacterMemoryBankIndex * CHARACTER_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetScreenMemoryOffset(){
	u8 vicScreenMemoryBankIndex = (chipMemoryControlRegister >> 4) & 0x0F;
	return vicScreenMemoryBankIndex * SCREEN_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetSpritePointersMemoryOffset(){
	return GetScreenMemoryOffset() + SCREEN_MEMORY_BANK_SIZE - NUMBER_OF_HARDWARE_SPRITES;
}


bool CMOS6569::IsSpriteEnabled(unsigned int spriteIndex){
	u8 spriteEnable = Peek(0xD015);
	return (spriteEnable & (1 << spriteIndex)) != 0;
}


u8 CMOS6569::GetSpritePointerValue(unsigned int spriteIndex){
	u16 address = mBus->GetVicMemoryBankStartAddress() + GetSpritePointersMemoryOffset() + spriteIndex;

	mBus->SetMode(eBusModeVic);
	return mBus->Peek(address);
}


u16 CMOS6569::GetSpriteDataMemoryOffset(unsigned int spriteIndex){
	return GetSpritePointerValue(spriteIndex) * HARDWARE_SPRITE_DATA_BLOCK_SIZE;
}


u16 CMOS6569::GetSpriteXPosition(unsigned int spriteIndex){
	u8 spriteXPositionMsbs = Peek(0xD010);
	u8 spriteXPositionLsb = Peek(0xD000 + (spriteIndex * 2));

	u16 spriteXPosition = spriteXPositionLsb;
	if ((spriteXPositionMsbs & (1 << spriteIndex)) != 0){
		spriteXPosition += 0x100;
	}

	return spriteXPosition;
}


u8 CMOS6569::GetSpriteYPosition(unsigned int spriteIndex){
	return Peek(0xD001 + (spriteIndex * 2));
}


unsigned int CMOS6569::GetSpriteHorizontalScale(unsigned int spriteIndex){
	u8 spriteXExpands = Peek(0xD01D);
	return ((spriteXExpands & (1 << spriteIndex)) != 0) ? 2 : 1;
}


unsigned int CMOS6569::GetSpriteVerticalScale(unsigned int spriteIndex){
	u8 spriteYExpands = Peek(0xD017);
	return ((spriteYExpands & (1 << spriteIndex)) != 0) ? 2 : 1;
}


u8 CMOS6569::GetSpriteWidth(unsigned int spriteIndex){
	return HARDWARE_SPRITE_WIDTH * GetSpriteHorizontalScale(spriteIndex);
}


u8 CMOS6569::GetSpriteHeight(unsigned int spriteIndex){
	return HARDWARE_SPRITE_HEIGHT * GetSpriteVerticalScale(spriteIndex);
}


bool CMOS6569::IsSpriteOnLine(unsigned int spriteIndex, unsigned int lineNumber){
	u8 spriteYPosition = GetSpriteYPosition(spriteIndex);
	return lineNumber >= spriteYPosition && lineNumber < (spriteYPosition + GetSpriteHeight(spriteIndex));
}


void CMOS6569::RegisterHWScreen(CVICHWScreen* screen){
	mRenderer = screen;
}


void CMOS6569::HWNeedsRedraw(){
}

