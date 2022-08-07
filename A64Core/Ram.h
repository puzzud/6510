/*
 *  Ram.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef RAM_H
#define RAM_H

#include "Common.h"
#include "Device.h"

#define MAXRAM 1024 * 64

class CRam : public CDevice{
	private:
		u8 mRam[MAXRAM];
		CRam& operator+=(CRam&);
	
	protected:
		void GetDefaultAddressRange(u16 *from, u16 *to);
	public:
		CRam();
		virtual ~CRam(){}
		u8 GetDeviceID();
		u8 Peek(u16 address);
		int Poke(u16 address, u8 val); 
		int PokeBlock(u16 address, u8* memory, u16 size);
		int LoadBasic(char* fname);
		int LoadApp(char* fname);
		int LoadBinary(char* fname, u16 address);
};


#endif //RAM_H
