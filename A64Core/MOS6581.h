/*
 *  MOS6581.h 
 *  A64Emulator
 *  
 *  6581 Audio Chip (SID) Emulator  
 *
 *  Created by Bruno Keymolen on 06/07/08.
 *  Copyright 2008 Bruno Keymolen. All rights reserved.
 *
 */

#ifndef MOS6581_H
#define MOS6581_H

#include "Common.h"
#include "Device.h"
#include "Bus.h"


#define SID_NUMBER_OF_REGISTERS           30


class CMOS6581 : public CDevice{
	private:
		u8 mRegs[SID_NUMBER_OF_REGISTERS];
	protected:
	public:
		CMOS6581();
		u8 GetDeviceID();

		u8 Peek(u16 address);
		int Poke(u16 address, u8 val);
};

#endif //MOS6581_H
