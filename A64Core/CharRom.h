/*
 *  CharRom.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CHARROM_H
#define CHARROM_H

#include "Common.h"
#include "Rom.h"

#define CHARROMSTART  0xD000
#define CHARROMEND    0xDFFF
#define CHARROMSIZE   (CHARROMEND - CHARROMSTART + 1) 

class CCharRom : public CRom{
	private:
		CCharRom& operator+=(CCharRom&);
	
	protected:
		void GetDefaultAddressRange(u16 *from, u16 *to);
	public:
		CCharRom();
		virtual ~CCharRom(){}
		
		u8 GetDeviceID();
		u8 Peek(u16 address);
		int Poke(u16 address, u8 val); 
	
};

#endif //CHARROM_H
