/*
 *  MOS6526.cpp
 *  A64Mac
 *
 *  Created by Bruno Keymolen on 13/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "MOS6526_CIA1.h"
#include <sys/time.h>


static int getTimeNow(){
	struct timeval t;
	gettimeofday(&t, NULL);	
	return ((t.tv_sec * 1000000) + t.tv_usec); //mHiresTime->GetMicrosecondsLo();
}

CMOS6526CIA1::CMOS6526CIA1(BKE_MUTEX mutex){
	mMutex = mutex;

	irq = false;

	pra = 0xFF;
	prb = 0xFF;
	ddra = 0xFF;
	ddrb = 0xFF;
	
	mBus = CBus::GetInstance();
	mBus->Register(eBusCia1,this, 0xDC00, 0xDCFF);

	memset(keyboardMatrix, 0xFF, sizeof(keyboardMatrix));
	memset(joystickStates, 0XFF, sizeof(joystickStates));

	timerAEnabled = false;
	timerACompleted = false;
	timerAIrqEnabled = false;

	prevIrqTime = getTimeNow();
}

CMOS6526CIA1::~CMOS6526CIA1(){
}

u8 CMOS6526CIA1::GetDeviceID(){
	return eBusCia1;
}

void CMOS6526CIA1::Cycle(){
	if(timerAEnabled){
		int timeNow = getTimeNow();

		if(timeNow - prevIrqTime >= 16700){ // 1/60 second to spoof default kernal setting.
			timerACompleted = true;
			
			// TODO: Manage excess remainder for next call.
			prevIrqTime = timeNow;
		}
	}

	// NOTE: Does this condition require timerAEnabled?
	irq = timerAIrqEnabled && timerAEnabled && timerACompleted;
}

u8 CMOS6526CIA1::Peek(u16 address){
	if(address == 0xDC00){
		// TODO: Checking ddra != 0
		// doesn't work in practice. Why?
		return joystickStates[1];
	}else if(address == 0xDC01){
		if(ddrb != 0){
			return 0xFF;
		}

		if(pra == 0xFF){
			// pra 0xFF with ddrb 0 will block keyboard input.
			// But it doesn't block joystick 1.
			// If joystick states is not returned, it was
			// good to return 0xFF.
			return joystickStates[0];
		}

		u8 row = 0;

		if (pra == 0x00){
			// Scan all rows together.
			// Checking for any key presses.
			u8 val = 0xFF;

			for(; row < 8; ++row){
				val &= keyboardMatrix[row];
			}

			// Joystick 1 input gets tied into this.
			val &= joystickStates[0];

			return val;
		}
		
		u8 val = (~pra) & 0xFF;
		while (val >>= 1){
			++row;
		}

		return keyboardMatrix[row] & joystickStates[0];
	}else if(address == 0xDC02){
		return ddra;
	}else if(address == 0xDC03){
		return ddrb;
	}else if(address == 0xDC0D){
		//u8 val = timerACompleted ? 1 : 0;

		timerACompleted = false; // Reset when read.

		// TODO: Should this return something special (not zero)?
	}

	return 0;
}


int CMOS6526CIA1::Poke(u16 address, u8 val){
	if(address == 0xDC00){
		if(ddra == 0xFF){
			pra = val;
		}
	}else if(address == 0xDC01){
		if(ddrb == 0xFF){
			prb = val;
		}
	}else if(address == 0xDC02){
		ddra = val;
	}else if(address == 0xDC03){
		ddrb = val;
	}else if(address == 0xDC0D){
		if((val & 0x80) != 0){
			timerAIrqEnabled = (val & 0x01) != 0;
		}else{
			timerAIrqEnabled = (val & 0x01) == 0;
		}

		//cout << int(timerAIrqEnabled) << endl;
	}else if(address == 0xDC0E){
		timerAEnabled = (val & 0x01) != 0;
		if(timerAEnabled){
			prevIrqTime = getTimeNow();
		}
	}

	return 0;
}	

bool CMOS6526CIA1::GetIRQ(){
	return irq;
}


CCIA1HWController* CMOS6526CIA1::GetHWController(){
	return mController;
}


void CMOS6526CIA1::RegisterHWController(CCIA1HWController* controller){
	// Detach previous one from CIA1.
	if (mController != NULL && mController != controller){
		mController->SetCia1(NULL);
	}
	
	mController = controller;

	// Attach new one to CIA1.
	if (controller != NULL){
		controller->SetCia1(this);
	}
}


int CMOS6526CIA1::AddKeyStroke(char c){
	BKE_MUTEX_LOCK(mMutex);
	
	u8 bufPos = mBus->Peek(0xC6);
	if(bufPos >= 10){
		return -1;
	}
	mBus->PokeDevice(eBusRam, 0x0277+bufPos, c);
	mBus->Poke(0xC6, bufPos+1);
	
	BKE_MUTEX_UNLOCK(mMutex);

	return 0;
}

int CMOS6526CIA1::SetKeyState(u8 row, u8 column, bool keyStateDown){
	if(row > 7){
		return -1;
	}

	// Keyboard matrix is active low.
	if(keyStateDown){
		keyboardMatrix[row] &= ~(1 << column);
	}else{
		keyboardMatrix[row] |= (1 << column);
	}

	return 0;
}

int CMOS6526CIA1::SetJoystickState(u8 joystickIndex, u8 buttonIndex, bool buttonStateDown){
	if(joystickIndex > 1){
		return -1;
	}

	// Joystick state is active low.
	if(buttonStateDown){
		joystickStates[joystickIndex] &= ~(1 << buttonIndex);
	}else{
		joystickStates[joystickIndex] |= (1 << buttonIndex);
	}

	return 0;
}
