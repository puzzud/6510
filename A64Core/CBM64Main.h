/*
 *  CBM64Main.h
 *  A64Mac
 *
 *  Created by Bruno Keymolen on 12/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef CBM64MAIN_H
#define CBM64MAIN_H

#include <iostream>
#include "MOS6510.h"
#include "MOS6569.h"
#include "Bus.h"
#include "Ram.h"
#include "Rom.h"
#include "BasicRom.h"
#include "KernalRom.h"
#include "MOS6526_CIA1.h"
#include "MOS6526_CIA2.h"
#include "MOS6581.h"
#include "CharRom.h"
#include "General.h"
#include "Watcher.h"
#include <thread>

class CBM64Main{
	private:
		CBus* mBus;
		CMOS6569* mVic; //Video Chip, VIC-II (6569)
		CRam* mRam;
		CBasicRom* mBasicRom;
		CKernalRom* mKernalRom;
		CCharRom* mCharRom;
		CMOS6510* mProcessor;
		CMOS6526CIA1* mCia1;
		CMOS6526CIA2* mCia2;
		CMOS6581* mSid;
		
		BKE_THREAD mCBM64Thread;
		BKE_MUTEX mMutex;
	protected:
		bool requireLoadedRoms;

		int LoadRoms();
	public:
		CBM64Main() : requireLoadedRoms(true){}

		int Init();
        int Cycle();
		int Stop();

		int SetDisassemble(int d);
		int GetDisassemble();
		
		CMOS6510* GetCpu();
		CMOS6569* GetVic();
		CMOS6526CIA1* GetCia1();
		CMOS6526CIA2* GetCia2();
		CCharRom* GetCharRom();

		void SetRequireLoadedRoms(bool require);

		int FlashRom(CRom* rom, const char* fname);
		int LoadApp(char* fname);
		int LoadAppWithoutBasic(const char* fname);
		int LoadBasic(char* fname);
		void SetupPostKernalConfig();
		
		void SetWatcher(CWatcher* watcher);
		void SetHiresTimeProvider(CHiresTime* hTime);

        uint64_t GetCycles();
};

#endif
