/*
 *  Rom.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef ROM_H
#define ROM_H

#include "Common.h"
#include "Device.h"

class CRom : public CDevice{
	private:
	protected:
		u8* mRom;
		uint64_t size;
	public:
		CRom(uint64_t size);
		virtual ~CRom();
	
		int Flash(u8* memory);
};

#endif //ROM_H
