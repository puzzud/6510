/*
 *  MOS6581.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 06/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MOS6581.h"
#include "Util.h"

#include <stdlib.h>
#include <sys/time.h>

CMOS6581::CMOS6581(){
	memset(mRegs, 0, sizeof(mRegs));

	srand(time(NULL));

	CBus::GetInstance()->Register(eBusSid, this, 0xD400, 0xD7FF);
}


u8 CMOS6581::GetDeviceID(){
	return eBusSid;
}


u8 CMOS6581::Peek(u16 address){
	if(address >= 0xD400 && address <= 0xD41C){
		// TODO: Implement mirroring.
		if(address == 0xD41B){
			//u8 voice3ControlRegister = mRegs[0xD412-0xD400];
			
			// NOTE: When oscillator 3 is not set to a noise waveform,
			// it's not as random.
			// Also, the rate at which these number change is based
			// on the frquency of oscillator 3 as well.
			// So, just using rand() is a rough way of implementing
			// this register.
			/*
			if((voice3ControlRegister & 0x80) == 0){
				debug_out << "SID RNG peeked without noise waveform" << endl;
			}
			*/

			return (u8)(rand() % 255);
		}	

		return mRegs[address-0xD400];
	}

	return 0xff;
}


int CMOS6581::Poke(u16 address, u8 val){
	if(address >= 0xD400 && address <= 0xD41C){
		// TODO: Implement mirroring.
		if(address >= 0xD419 && address <= 0xD41C){
			// Read only.
			return 0;
		}

		mRegs[address-0xD400] = val;
	}

	return 0;
}
