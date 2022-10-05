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

	// Load ROMs first,
	// as CMOS6510 constructor
	// requires them to be loaded.
	mBasicRom = new CBasicRom();
	mKernalRom = new CKernalRom();
	mCharRom = new CCharRom();
	LoadRoms();

	mRam = new CRam();	

	mVic = new CMOS6569();	
	mProcessor = new CMOS6510(mMutex);
	mCia1 = new CMOS6526CIA1(mMutex);
	mCia2 = new CMOS6526CIA2(mMutex);
	mSid = new CMOS6581();

    return 0;
}


int CBM64Main::Cycle(){
	mProcessor->SetIRQ(mVic->GetIRQ() || mCia1->GetIRQ());
    int cycles = mProcessor->Cycle();

	for (int i = 0; i < cycles; ++i){
		mVic->Cycle();
		mCia1->Cycle();
		mCia2->Cycle();
	}

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

CMOS6510* CBM64Main::GetCpu(){
	return mProcessor;
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


void CBM64Main::SetRequireLoadedRoms(bool require){
	requireLoadedRoms = require;
}


int CBM64Main::FlashRom(CRom* rom, const char* fname){
	ifstream file(fname, ios::in|ios::binary|ios::ate);
	if (!file.is_open()){
		return -1;
	}

	unsigned int fileSize = file.tellg();
	file.seekg(0, ios::beg);

	u8* m = new u8[fileSize];
	file.read((char*)m, fileSize);
	file.close();

	int result = rom->Flash(m);

	delete[] m;
	m = NULL;

	return result;
}


int CBM64Main::LoadRoms(){
#ifndef EMBEDDED_ROMS
	if (FlashRom(mBasicRom, LOCATION_ROMS "BASIC.ROM") != 0){
		if (requireLoadedRoms){
			cout << "BASIC.ROM could not be loaded." << endl;
			exit(-1);
		}
	}

	if (FlashRom(mCharRom, LOCATION_ROMS "CHAR.ROM") != 0){
		if (requireLoadedRoms){
			cout << "BASIC.ROM could not be loaded." << endl;
			exit(-1);
		}
	}

	if (FlashRom(mKernalRom, LOCATION_ROMS "KERNAL.ROM") != 0){
		if (requireLoadedRoms){
			cout << "BASIC.ROM could not be loaded." << endl;
			exit(-1);
		}
	}
#endif

	return 0;
}


int CBM64Main::LoadApp(char* fname){
	return mRam->LoadApp(fname);
}


int CBM64Main::LoadAppWithoutBasicFromMemory(u8* m, unsigned int fileSize){
	cout << "File Size: " << fileSize << endl;
	
	u16 startAddress = (m[1] << 8) | m[0];
	cout << std::hex << "Start Address:  " << startAddress << endl << std::dec;

	mRam->PokeBlock(startAddress, m + 2, fileSize - 2);

	return 0;
}


int CBM64Main::LoadAppWithoutBasic(const char* fname){
	ifstream file(fname, ios::in|ios::binary|ios::ate);
	if (file.is_open()){
		unsigned int fileSize = file.tellg();
		cout << "File Size: " << fileSize << endl;
		file.seekg(0, ios::beg);

		u16 startAddress;
		file.read((char*)&startAddress, 2);
		cout << std::hex << "Start Address:  " << startAddress << endl << std::dec;

		u8* m = new u8[fileSize - 2];
		file.read((char*)m, fileSize - 2);
		file.close();
		
		mRam->PokeBlock(startAddress, m, fileSize - 2);

		delete[] m;
		m = NULL;
	}else{
		cout << "Could not load file : " << fname << endl;
		return -1;
	}

	return 0;
}


int CBM64Main::LoadBasic(char* fname){
	return mRam->LoadBasic(fname);
}


void CBM64Main::SetupPostKernalConfig(){
	// $FCE2:START(RESET)
	// ---
	// SEI
	// TXS from X=$FF
	// CLD
	//mVic->Poke(0xD016, 0); // Overwritten later.
	
	// $FDA3:IOINIT
	// ---

	// Kill interrupts.
	mCia1->Poke(0xDC03, 0x7F);
	mCia2->Poke(0xDD03, 0x7F);
	mCia1->Poke(0xDC00, 0x7F);
	mCia1->Poke(0xDC0E, 0x08);
	mCia2->Poke(0xDD0E, 0x08);
	mCia1->Poke(0xDC0F, 0x08);
	mCia2->Poke(0xDD0F, 0x08);
	mCia1->Poke(0xDC03, 0);
	mCia2->Poke(0xDD03, 0);

	// Volume.
	mSid->Poke(0xD418, 0);

	// Keyboard.
	mCia1->Poke(0xDC02, 0xFF);
	mCia2->Poke(0xDD00, 0x3F);

	// System banks.
	mBus->Poke(0x0001, 0xE7);
	mBus->Poke(0x0000, 0x2F);

	// Keyboard scan.
	// "SIXTY" (NTSC), "SIXTYP" (PAL) is 0x94,0x42.
	mCia1->Poke(0xDC04, 0x25);
	mCia1->Poke(0xDC05, 0x40);
	
	// $FD52
	// ---
	// Clear zeropage, page 2, 3.

	// $FF5B:CINT(Screen Initialize).
	// ---
	// VIC values.
	// https://www.pagetable.com/c64ref/c64disasm/#E5A0
	u8 vicInitValues[] = {
		0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
		0x1B,0,0,0,0,0x08,0,0x14,0,0,0,0,0,0,0,
		14,6,1,2,3,4,0,1,2,3,4,5,6,7
	};

	for (unsigned int i = 0; i < sizeof(vicInitValues); ++i){
		mVic->Peek(0xD000+i);
	}

	// Clear screen memory.

	// CLI
}


void CBM64Main::SetWatcher(CWatcher* watcher){
	mProcessor->SetWatcher(watcher);
}


void CBM64Main::SetHiresTimeProvider(CHiresTime* hTime){
	mProcessor->SetHiresTime(hTime);
}

uint64_t CBM64Main::GetCycles(){
    return mProcessor->GetCycles();
}
