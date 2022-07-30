/*
 *  CBM64Main.cpp
 *  A64Mac
 *
 *  Created by Bruno Keymolen on 12/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "CBM64Main.h"

int CBM64Main::Init(){
	std::cout << "---------------------------------\n";
    std::cout << "Commodore64 Emulator\n";
    std::cout << "Copyright (C)2008, Bruno Keymolen\n";
    std::cout << "---------------------------------\n";
	
	BKE_MUTEX_CREATE(mMutex);	 
	
	mBus = CBus::GetInstance();
	mVic = new CMOS6569();	
	mRam = new CRam();	
	mBasicRom = new CBasicRom();
	mKernalRom = new CKernalRom();
	mProcessor = new CMOS6510(mMutex);
	mCia1 = new CMOS6526CIA1(mMutex);
	mCia2 = new CMOS6526CIA2(mMutex);
	mCharRom = new CCharRom();
	mSid = new CMOS6581();
	
    return 0;
}


int CBM64Main::Cycle(){
    mVic->Cycle();
	int cycles = mProcessor->Cycle();
    mCia1->Cycle();
    mCia2->Cycle();
    return cycles;
}

int CBM64Main::Stop(){	
	delete mProcessor;
	delete mVic;
	delete mRam;
	delete mKernalRom;
	delete mBasicRom;
	delete mCia1;
	delete mCia2;
	delete mCharRom;
	delete mSid;
	return 0;
}

CMOS6569* CBM64Main::GetVic(){
	return mVic;
}

CMOS6526CIA1* CBM64Main::GetCia1(){
	return mCia1;
}

CMOS6526CIA2* CBM64Main::GetCia2(){
	return mCia2;
}

CCharRom* CBM64Main::GetCharRom(){
	return mCharRom;
}

int CBM64Main::SetDisassemble(int d){
	mProcessor->mDisassemble = d;
	return mProcessor->mDisassemble;
}


int CBM64Main::GetDisassemble(){
	return mProcessor->mDisassemble;
}


int CBM64Main::LoadApp(char* fname){
	return mRam->LoadApp(fname);
}


int CBM64Main::LoadBasic(char* fname){
	return mRam->LoadBasic(fname);
}


void CBM64Main::SetHiresTimeProvider(CHiresTime* hTime){
	mProcessor->SetHiresTime(hTime);
}

uint64_t CBM64Main::GetCycles(){
    return mProcessor->GetCycles();
}



