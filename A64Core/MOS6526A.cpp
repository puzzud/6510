/*
 *  MOS6526.cpp
 *  A64Mac
 *
 *  Created by Bruno Keymolen on 13/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MOS6526A.h"


CMOS6526A::CMOS6526A(BKE_MUTEX mutex){
	mMutex = mutex;
	mBus = CBus::GetInstance();
	mBus->Register(eBusCia1,this, 0xDC00, 0xDC0F);
}

CMOS6526A::~CMOS6526A(){
}

u8 CMOS6526A::GetDeviceID(){
	return eBusCia1;
}


u8 CMOS6526A::Peek(u16 address){
cout << "6526A Peek:" << (int)address << endl;
	return mBus->PeekDevice(eBusRam,address);
}


int CMOS6526A::Poke(u16 address, u8 val){
cout << "6526A Poke:" << (int)address << ", m=" << (int)val << endl;
	mBus->PokeDevice(eBusRam,address,val);
	return 0;
}	

int CMOS6526A::AddKeyStroke(char c){
	BKE_MUTEX_LOCK(mMutex);
	
	u8 bufPos = mBus->Peek(0xC6);
cout << (int)bufPos;	
	if(bufPos >= 10){
		return -1;
	}
	mBus->PokeDevice(eBusRam, 0x0277+bufPos, c);
	mBus->Poke(0xC6, bufPos+1);
	
	BKE_MUTEX_UNLOCK(mMutex);

	return 0;
}


