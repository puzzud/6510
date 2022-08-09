/*
 *  KernalRom.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "KernalRom.h"
#include "Bus.h"

#include <iostream>
#include <fstream>
#include "General.h"

#ifdef EMBEDDED_ROMS
extern char _binary_KERNAL_ROM_start[];
extern char _binary_KERNAL_ROM_end[];
#endif


CKernalRom::CKernalRom():CRom(KERNALROMSIZE){
#ifdef EMBEDDED_ROMS
    std::cout << "adding embedded rom" << std::endl;
    int len = (uint64_t)(&_binary_KERNAL_ROM_end) - (uint64_t)(&_binary_KERNAL_ROM_start);
    if (KERNALROMSIZE != len) {
        std::cout << "kernal rom error... embedded len: " << len << " expect: " << KERNALROMSIZE << std::endl;
        exit(-1);
    }
    memcpy(mRom, _binary_KERNAL_ROM_start, KERNALROMSIZE);
#endif

	CBus::GetInstance()->Register(eBusKernalRom, this, KERNALROMSTART, KERNALROMEND);

}




u8 CKernalRom::GetDeviceID(){
	return eBusKernalRom;
}

u8 CKernalRom::Peek(u16 address){
	return *(mRom+(address - KERNALROMSTART));
}

int CKernalRom::Poke(u16 address, u8 val){
	//Can not poke into ROM
	return -1;
}
