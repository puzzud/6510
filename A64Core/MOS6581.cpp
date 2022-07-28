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


CMOS6581::CMOS6581(){
	CBus::GetInstance()->Register(eBusSid, this, 0xD400, 0xD7FF);
}


u8 CMOS6581::GetDeviceID(){
	return eBusSid;
}


u8 CMOS6581::Peek(u16 address){
	if(address == 0xD418){
		// SIGVOL
		return 0;
	}
	return 0xff;
}


int CMOS6581::Poke(u16 address, u8 val){
	return 0;
}
