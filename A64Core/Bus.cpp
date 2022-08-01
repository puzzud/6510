/*
 *  Bus.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

/*
	Memory Banking
	
	0x0000 : Masked IO Register
				Bit 0:LORAM
				    1:HIRAM
					2:CHAREN
	0x0001 : IO Port
	
*/


#include "Bus.h"
#include "Common.h"

CBus* CBus::_instance = 0;

CBus* CBus::GetInstance(){
	if(_instance == NULL){
		_instance = new CBus();
	}
	return _instance;
}


CBus::CBus(){
	memset(&mIO, 0, sizeof(sBusDevice));
	memset(&mVic, 0, sizeof(sBusDevice));
	memset(&mCharRom, 0, sizeof(sBusDevice));
	memset(&mRam, 0, sizeof(sBusDevice));
	memset(&mBasicRom, 0, sizeof(sBusDevice));
	memset(&mKernalRom, 0, sizeof(sBusDevice));
	memset(&mCia1, 0, sizeof(sBusDevice));
	memset(&mCia2, 0, sizeof(sBusDevice));
	memset(&mSid, 0, sizeof(sBusDevice));

	mLoRam = true;
	mHiRam = true;
	mCharen = true;

	//IO Ports, for simplification the ports are put here instead of at the processor
	mPort0 = 0xFF;
	mPort1 = 0xFF;

	mMemoryMode = eBusModeProcesor;

	// TODO: No single device instance associated with IO.
	// PeekDevice & PokeDevice should be enhanced to resolve the appropriate device,
	// if called with eBusIO.
	Register(eBusIO, NULL, 0xD000, 0xDFFF);
}


void CBus::Register(e_BusDevice devid, CDevice* device, u16 fromAddress, u16 toAddress){

	switch(devid){
		case eBusIO:
			mIO.device=device;
			mIO.fromAddress = fromAddress;
			mIO.toAddress = toAddress;
			break;		
		case eBusVic:
			mVic.device=device;
			mVic.fromAddress = fromAddress;
			mVic.toAddress = toAddress;
			break;
		case eBusCharRom:
			mCharRom.device=device;
			mCharRom.fromAddress = fromAddress;
			mCharRom.toAddress = toAddress;
			break;		
		case eBusRam:
			mRam.device=device;
			mRam.fromAddress = fromAddress;
			mRam.toAddress = toAddress;
			break;
		case eBusBasicRom:
			mBasicRom.device=device;
			mBasicRom.fromAddress = fromAddress;
			mBasicRom.toAddress = toAddress;
			break;
		case eBusKernalRom:
			mKernalRom.device=device;
			mKernalRom.fromAddress = fromAddress;
			mKernalRom.toAddress = toAddress;
			break;
		case eBusCia1:
			mCia1.device=device;
			mCia1.fromAddress = fromAddress;
			mCia1.toAddress = toAddress;
			break;
		case eBusCia2:
			mCia2.device=device;
			mCia2.fromAddress = fromAddress;
			mCia2.toAddress = toAddress;
			break;
		case eBusSid:
			mSid.device=device;
			mSid.fromAddress = fromAddress;
			mSid.toAddress = toAddress;
			break;
		default:
			cout << "Error! 109" << endl;
			break;
	}

}


void CBus::SetMode(e_BusMode mode){
	mMemoryMode = mode;
}	


e_BusDevice CBus::GetDeviceIdFromAddress(u16 address){
	// NOTE: Assumes eBusModeProcesor.

	/* Basic ROM or RAM */
	if(address >= 0xA000 && address <= 0xBFFF){
		return (mHiRam && mLoRam) ? eBusBasicRom : eBusRam;
	}
	
	/* CharRom or IO or RAM */
	if(address >=0xD000 && address <= 0xDFFF){
		if(mHiRam || mLoRam){
			if(mCharen){
				if(address >= mVic.fromAddress && address <= mVic.toAddress){
					return eBusVic;
				}else if(address >= mSid.fromAddress && address <= mSid.toAddress){
					return eBusSid;
				}else if(address >= 0xD800 && address <= 0xDBFF){
					return eBusVic;
				}else if(address >= mCia1.fromAddress && address <= mCia1.toAddress){
					return eBusCia1;
				}else if(address >= mCia2.fromAddress && address <= mCia2.toAddress){
					return eBusCia2;
				}else{
					return eBusNone;
				}
			}else{
				return eBusCharRom;
			}
		}else{
			return eBusRam;
		}
	}
		
	/* Kernal ROM or RAM */	
	if(address >= 0xE000 && address <= 0xFFFF){
		return (mHiRam) ? eBusKernalRom : eBusRam;
	}
	
	/* Processor Ports */
	if(address == 0x0000 || address == 0x0001){
		return eBusCpu;
	}
	
	/* RAM */
	return eBusRam;
}


u8 CBus::Peek(u16 address){	
	switch(mMemoryMode){
		case eBusModeProcesor:{
			e_BusDevice deviceId = GetDeviceIdFromAddress(address);

			if(deviceId == eBusNone){
				cout << "Unmapped address peeked: 0x" << std::hex << address << " : 0x" << endl << std::dec;
				return 0xFF; // NOTE: Return 0xFF to maintain previous behavior of accessing through VIC.
			}else if(deviceId == eBusCpu){
				if(address == 0x0000){
					return 0x2F;
				}else if(address == 0x0001){
					// Fall back to RAM for now.
					deviceId = eBusRam;
				}
			}

			if(deviceId != eBusNone){
				return PeekDevice(deviceId, address);
			}
		}
			break;
		case eBusModeVic:
			SetMode(eBusModeProcesor);
			if(address >= 0x1000 && address <= 0x1FFF){
				address = (0xD000 + (address - 0x1000));
				return mCharRom.device->Peek(address);
			}else if(address >= 0x9000 && address <= 0x9FFF){
				address = (0xD000 + (address - 0x9000));
				return mCharRom.device->Peek(address);
			}
			return mRam.device->Peek(address);
	}				
	return 0;	
}


void CBus::Poke(u16 address, u8 m){
	switch(mMemoryMode){
		case eBusModeProcesor:{
			e_BusDevice deviceId = GetDeviceIdFromAddress(address);

			if(deviceId == eBusNone){
				cout << "Unmapped address poked: 0x" << std::hex << address << " with 0x" << int(m) << endl << std::dec;
			}else if(deviceId == eBusCpu){
				if(address == 0x0000){	
					// Do nothing special for now.
				}else if(address == 0x0001){
					//IO Port
					u8 r0 = mPort1; //mRam.device->Peek(1);
					
					u8 ioMem = m;// (r0 & m);
					mPort1 = ioMem;
					
					mLoRam = mHiRam = mCharen = 0;
					if(ioMem & 1)mLoRam = 1;
					if(ioMem & 2)mHiRam = 1;
					if(ioMem & 4)mCharen = 1;
				}

				// Fall back to RAM (probably not correct).
				deviceId = eBusRam;
			}

			if(deviceId != eBusNone){
				if(deviceId == eBusBasicRom || deviceId == eBusCharRom || deviceId == eBusKernalRom){
					// For pokes, fall down into RAM.
					deviceId = eBusRam;
				}

				PokeDevice(deviceId,address,m);
			}
		}
			break;
		case eBusModeVic:
			SetMode(eBusModeProcesor);	
			mRam.device->Poke(address, m);
			break;
		}
}


u16 CBus::Peek16(u16 address){	
	u16 ret = 0;
	ret = Peek(address+1) << 8;
	ret = ret | Peek(address);
	return ret;
}

void CBus::Poke16(u16 address, u16 m){
	Poke(address, ((u8*)&m)[0]);
	Poke(address+1, ((u8*)&m)[1]);
}


void CBus::PokeDevice(u8 deviceID, u16 address, u8 m){
	
	switch(deviceID){
		//case eBusIO:
		//	mIO.device->Poke(address, m);
		//	break;		
		case eBusVic:
			mVic.device->Poke(address, m);
			break;		
		case eBusRam:
			mRam.device->Poke(address, m);
			break;		
		case eBusBasicRom:
			mBasicRom.device->Poke(address, m);
			break;		
		case eBusKernalRom:
			mKernalRom.device->Poke(address, m);
			break;		
		case eBusCharRom:
			mCharRom.device->Poke(address, m);
			break;		
		case eBusCia1:
			mCia1.device->Poke(address, m);
			break;		
		case eBusCia2:
			mCia2.device->Poke(address, m);
			break;			
		case eBusSid:
			mSid.device->Poke(address, m);
			break;
		default:
			cout << "Error! 1239" << endl;
			break;
	}
}

u8 CBus::PeekDevice(u8 deviceID, u16 address){
	switch(deviceID){
		//case eBusIO:
		//	return mIO.device->Peek(address);
		//	break;		
		case eBusVic:
			return mVic.device->Peek(address);
			break;		
		case eBusRam:
			return mRam.device->Peek(address);
			break;		
		case eBusBasicRom:
			return mBasicRom.device->Peek(address);
			break;		
		case eBusKernalRom:
			return mKernalRom.device->Peek(address);
			break;		
		case eBusCharRom:
			return mCharRom.device->Peek(address);
			break;		
		case eBusCia1:
			return mCia1.device->Peek(address);
			break;		
		case eBusCia2:
			return mCia2.device->Peek(address);
			break;		
		case eBusSid:
			return mSid.device->Peek(address);
			break;
		default:
			cout << "Error! 1239" << endl;
			break;
	}
	return 0;
}

u16 CBus::GetVicMemoryBankStartAddress(){
	u8 vicBankIndex = ~PeekDevice(eBusCia2,0xDD00) & 0x03;
	return vicBankIndex * 0x4000;
}
