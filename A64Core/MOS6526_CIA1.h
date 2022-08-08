/*
 *  MOS6526.h
 *  A64Mac
 *  
 *  CIA 6526 Complex Interface Adapter Emulator  
 *
 *  Created by Bruno Keymolen on 13/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

// https://www.c64-wiki.com/wiki/CIA

#ifndef MOS6569CIA1_H
#define MOS6569CIA1_H

#include "Common.h"
#include "Device.h"
#include "Bus.h"


class CMOS6526CIA1;


class CCIA1HWController{
public:
	CCIA1HWController(){
		cia1 = NULL;
	}
	void SetCia1(CMOS6526CIA1* cia1){this->cia1 = cia1;}

	// running reference parameter allows input
	// processing to signal that processed
	// input has indicated termination.
	virtual void Process(bool& running){};
protected:
	CMOS6526CIA1* cia1;
};


class CMOS6526CIA1 : public CDevice{
private:
	bool irq;

	u8 pra;
	u8 prb;
	u8 ddra;
	u8 ddrb;

	bool timerAEnabled;
	bool timerACompleted;
	bool timerAIrqEnabled;
	int prevIrqTime;

	u8 keyboardMatrix[8];
	u8 joystickStates[2];

	CBus* mBus;
	CCIA1HWController* mController;

	BKE_MUTEX mMutex;
protected:
public:
	CMOS6526CIA1(BKE_MUTEX mutex);
	virtual ~CMOS6526CIA1();

    void Cycle();

	u8 GetDeviceID();
	u8 Peek(u16 address);
	int Poke(u16 address, u8 val); 	

	bool GetIRQ();

	// Hardware dependent calls
	CCIA1HWController* GetHWController();
	void RegisterHWController(CCIA1HWController* controller);
	int AddKeyStroke(char c);

	int SetKeyState(u8 row, u8 column, bool keyStateDown);
	int SetJoystickState(u8 joystickIndex, u8 buttonIndex, bool buttonStateDown);
};


#endif //MOS6569A_H
