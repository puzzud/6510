/*
 *  BasicRom.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "BasicRom.h"
#include <iostream>
#include <fstream>
#include "Bus.h"
#include "General.h"


#ifdef EMBEDDED_ROMS
extern char _binary_BASIC_ROM_start[];
extern char _binary_BASIC_ROM_end[];
#endif

CBasicRom::CBasicRom():CRom(BASICROMSIZE){
#ifdef EMBEDDED_ROMS
    std::cout << "adding embedded rom" << std::endl;
    int len = (uint64_t)(&_binary_BASIC_ROM_end) - (uint64_t)(&_binary_BASIC_ROM_start);
    if (BASICROMSIZE != len) {
        std::cout << "basic rom error... embedded len: " << len << " expect: " << BASICROMSIZE << std::endl;
        exit(-1);
    }
    memcpy(mRom, _binary_BASIC_ROM_start, BASICROMSIZE);
#endif	
	CBus::GetInstance()->Register(eBusBasicRom, this, BASICROMSTART, BASICROMEND);
	
}



u8 CBasicRom::GetDeviceID(){
	return eBusBasicRom;
}

u8 CBasicRom::Peek(u16 address){
	return *(mRom+(address - BASICROMSTART));
}

int CBasicRom::Poke(u16 address, u8 val){
	//Can not poke into ROM
	CBus::GetInstance()->PokeDevice(eBusRam,address,val);

	return -1;
}
