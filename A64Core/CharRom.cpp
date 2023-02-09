/*
 *  CharRom.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "CharRom.h"
#include <iostream>
#include <fstream>
#include "Bus.h"
#include "General.h"

#ifdef EMBEDDED_ROMS
extern char _binary_CHAR_ROM_start[];
extern char _binary_CHAR_ROM_end[];
#endif


CCharRom::CCharRom():CRom(CHARROMSIZE){
#ifdef EMBEDDED_ROMS
    debug_out << "adding embedded rom" << std::endl;
    int len = (uint64_t)(&_binary_CHAR_ROM_end) - (uint64_t)(&_binary_CHAR_ROM_start);
    if (CHARROMSIZE != len) {
        debug_out << "char rom error... embedded len: " << len << " expect: " << CHARROMSIZE << std::endl;
        exit(-1);
    }
    memcpy(mRom, _binary_CHAR_ROM_start, CHARROMSIZE);
#endif	
	CBus::GetInstance()->Register(eBusCharRom, this, CHARROMSTART, CHARROMEND);
	
}



u8 CCharRom::GetDeviceID(){
	return eBusCharRom;
}

u8 CCharRom::Peek(u16 address){

	return *(mRom+(address - CHARROMSTART));
}

int CCharRom::Poke(u16 address, u8 val){
	//Can not poke into ROM
	debug_out << "CCharRom Poke:" << (int)address << ", m=" << (int)val << endl;
	CBus::GetInstance()->PokeDevice(eBusRam,address,val);

	return -1;
}
