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
	chipMemoryControlRegister = 0;
	memset(mColorRam, 0, (0xDBFF-0xD800));
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
	if(address == 0xD012){
		return 0;
	}
	if(address == 0xD018){
		return chipMemoryControlRegister;
	}
	
	return 0xff;
}


int CMOS6569::Poke(u16 address, u8 val){
	if (address >= 0xD800 && address <= 0xDBFF){
		mColorRam[address - 0xD800] = val;
	}
	else if(address == 0xD018){
		chipMemoryControlRegister = val;
	}
	else if (address == 0xD020){
		mRenderer->SetBorderColor(val);
	}
	else if (address == 0xD021){
		mRenderer->SetBackgroundColor(val);
	}

	return 0;
}


u16 CMOS6569::GetCharacterMemoryOffset(){
	u8 vicCharacterMemoryBankIndex = (chipMemoryControlRegister >> 1) & 0x03;
	return vicCharacterMemoryBankIndex * CHARACTER_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetScreenMemoryOffset(){
	u8 vicScreenMemoryBankIndex = (chipMemoryControlRegister >> 4) & 0x0F;
	return vicScreenMemoryBankIndex * SCREEN_MEMORY_BANK_SIZE;
}


u16 CMOS6569::GetSpritePointersMemoryOffset(){
	return GetScreenMemoryOffset() + SCREEN_MEMORY_BANK_SIZE - 8; // 8 is # of sprites.
}


void CMOS6569::RegisterHWScreen(CVICHWScreen* screen){
	mRenderer = screen;
}


void CMOS6569::HWNeedsRedraw(){
}

