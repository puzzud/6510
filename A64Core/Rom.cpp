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


CRom::CRom(uint64_t size):CDevice(){
	this->size = size;
	mRom = new u8[size];
	memset(mRom, size, 0);
}


CRom::~CRom(){
#ifdef EMBEDDED_ROMS
	return;
#endif

	delete[] mRom;
}


int CRom::Flash(u8* memory){
#ifdef EMBEDDED_ROMS
	return -1;
#endif

	memcpy(mRom, memory, size);

	return 0;
}
