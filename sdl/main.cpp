/*
#include <chrono>
#include <ctime>
#include <atomic>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <map>
*/
#include <SDL2/SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../A64Core/CBM64Main.h"
#include "../A64Core/General.h"

#include "SDLHWScreen.h"
#include "SDLHWController.h"


extern void MainLoop(void);

#define M1_A_JumpToInitializeIntro               0x4000
#define M1_A_InitializeOver                      0x55b3
#define M1_A_CopyPlayerSpeciesGraphics           0x6427
#define M1_A_ProcessSettingsOver                 0x655b

#define M1_A_NumberOfHumanPlayers                0x43ff
#define M1_A_InitialGameDifficulty               0xe504

#define M2_A_JumpToGameInitialize                0x4000
#define M2_A_AddMessage                          0x437b
#define M2_A_DoShipArrive                        0x5563
#define M2_A_DoShipLeave                         0x557B
#define M2_A_DoRoundEvent                        0x59f4
#define M2_A_DoGoodAuction                       0x7f5b
#define M2_A_CalculateScoresDoStatusScreen       0x8c3e
#define M2_A_GameInitialize                      0x972b
#define M2_A_DoRound                             0x9771
#define M2_A_EndRoundEvent                       0x97f1
#define M2_A_DoLandGrant                         0x9bff
#define M2_A_DoDevelopmentTurn                   0xa637
#define M2_A_AuctionTrade                        0xb362
#define M2_A_DoRandomLandAuctions                0xbcfb
#define M2_A_DoPlayerLandAuction                 0xbd51

#define M2_A_RoundNumber                         0x0048
#define M2_A_UnmappedIoSpaceStart                0xdf12

#define MX_A_MessageColor                        0x0027
#define MX_A_MessageFont                         0x009e
#define MX_A_MessageXPos                         0x00b1
#define MX_A_MessageYPos                         0x00b7
#define MX_A_PlayerInputType                     0xe500
#define MX_A_PlayerColor                         0xe505
#define MX_A_PlayerSpecies                       0xe509

void BootM1();

void M1_SetInitialGameDifficulty(u8 gameDifficulty);
void M1_SetFakeIntroUserSettings();
void M1_ApplySettingsAndJumpToDataCopy();
void M1_LoadAndBootM2();

void MX_SetPlayerInputType(unsigned int playerIndex, u8 playerInputType);
void MX_SetPlayerColor(unsigned int playerIndex, u8 playerColor);
void MX_SetPlayerSpecies(unsigned int playerIndex, u8 playerSpecies);


class CustomWatcher;


CBM64Main* cbm64 = NULL;
CustomWatcher* watcher = NULL;

unsigned int CurrentModuleId;
unsigned int ScreenId;
unsigned int SubScreenId;
bool ShouldPlayIntro = false;
bool DelayBeforeM2Load = false;


class CustomWatcher : public CWatcher
{
private:
protected:
	virtual int GeneralReportAddressWatch(u16 address)
	{
		cout << "Watcher Address: " << std::hex << int(address) << std::dec << endl;

		return 0;
	};

	virtual int GeneralReportJumpWatch(u16 address, eWatcherJumpType jumpType)
	{
		cout << "Watcher Jump: " << std::hex << int(address) << std::dec << endl;

		return 0;
	};

	virtual int GeneralReportReadWatch(u16 address)
	{
		cout << "Watcher Read: " << std::hex << int(address) << std::dec << endl;

		return 0;
	}

	virtual int GeneralReportWriteWatch(u16 address)
	{
		cout << "Watcher Write: " << std::hex << int(address) << std::dec << endl;

		return 0;
	}

public:
	int M1_InitializeOver(u16 address)
	{
		M1_ApplySettingsAndJumpToDataCopy();

		return 1;
	}
	
	int M2_GameInitialize(u16 address)
	{
		cout << "GameInitialize" << endl;
		return 0;
	}

	int M2_DoRound(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 0;

		cout << "DoRound" << endl;
		return 0;
	}

	int M2_DoLandGrant(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 1;

		cout << "DoLandGrant" << endl;
		return 0;
	}

	int M2_DoPlayerLandAuction(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 2;

		cout << "DoPlayerLandAuction" << endl;
		return 0;
	}

	int M2_DoRandomLandAuctions(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 3;

		cout << "DoRandomLandAuctions" << endl;
		return 0;
	}

	int M2_DoDevelopmentTurn(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 4;

		cout << "DoDevelopmentTurn" << endl;
		return 0;
	}

	int M2_DoRoundEvent(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 5;

		cout << "DoRoundEvent" << endl;
		return 0;
	}

	int M2_DoShipArrive(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 5;

		cout << "DoShipArrive" << endl;
		return 0;
	}

	int M2_DoShipLeave(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 5;

		cout << "DoShipLeave" << endl;
		return 0;
	}

	int M2_EndRoundEvent(u16 address)
	{
		ScreenId = 0;
		SubScreenId = 6;

		cout << "EndRoundEvent" << endl;
		return 0;
	}

	int M2_DoGoodAuction(u16 address)
	{
		ScreenId = 1;
		SubScreenId = 0;

		cout << "DoGoodAuction" << endl;
		return 0;
	}

	int M2_AuctionTrade(u16 address)
	{
		ScreenId = 2;
		SubScreenId = 0;

		cout << "AuctionTrade" << endl;
		return 0;
	}

	int M2_CalculateScoresDoStatusScreen(u16 address)
	{
		ScreenId = 3;
		SubScreenId = 0;

		cout << "CalculateScoresDoStatusScreen" << endl;
		return 0;
	}

	int M2_AddMessage(u16 address)
	{
		auto bus = CBus::GetInstance();
		u8 messageXPos = bus->Peek(MX_A_MessageXPos);
		u8 messageYPos = bus->Peek(MX_A_MessageYPos);
		u8 messageFont = bus->Peek(MX_A_MessageFont);
		u8 messageColor = bus->Peek(MX_A_MessageColor);

		cout << "AddMessage: ";
		cout << "(" << int(messageXPos) << "," << int(messageYPos) << ")";
		cout << "[F:" << int(messageFont) << " " << "C:" << int(messageColor) << "]";
		cout << endl;

		auto cpu = cbm64->GetCpu();
		u8 aReg = cpu->GetA();
		u8 yReg = cpu->GetY();
		u16 stringAddress = aReg | ((u16)yReg << 8);

		bool terminator;
		u8 character;
		do
		{
			character = bus->Peek(stringAddress++);

			// Decode character code.
			terminator = (character & 0x80) == 0;
			character = char(character & ~0x80);

			if (character < 32)
			{
				if (character == '\r')
				{
					cout << endl;
					continue;
				}

				cout << "[" << int(character) << "]";
				continue;
			}

			cout << character;
		}
		while(!terminator);

		cout << endl;

		return 0;
	}

	int M1_ProcessSettingsOver(u16 address)
	{
		M1_LoadAndBootM2();

		return 1;
	}

	int M2_RoundNumber(u16 address)
	{
		auto bus = CBus::GetInstance();
		u8 roundNumber = bus->Peek(address);

		cout << "RoundNumber" << ": " << int(roundNumber) << endl;

		return 0;
	}

	int M2_UnmappedIoSpaceStart(u16 address)
	{
		cout  << std::hex;
		cout << "Watcher Write: " << int(address);
		cout << " before PC: " << int(cbm64->GetCpu()->GetPC()) << endl;
		cout << std::dec;

		return 0;
	}
};


int main(int argc, char* argv[]) {
    cbm64 = new CBM64Main();
    cbm64->Init();

    // Subscribe host hardware related VIC code.
    SDLHWScreen hwScreen;
    cbm64->GetVic()->RegisterHWScreen(&hwScreen);
	hwScreen.Init();

	// Subscribe host hardware related CIA1 code.
    SDLHWController hwController;
    cbm64->GetCia1()->RegisterHWController(&hwController);
	hwController.Init();
	hwController.SetKeyboardMode(eKeyboardModeJoystick1);

	cbm64->SetupPostKernalConfig();

	watcher = new CustomWatcher();
	cbm64->SetWatcher(watcher);

	BootM1();

	MainLoop();

	hwController.Shutdown();
	hwScreen.Shutdown();

	delete watcher;
	watcher = NULL;

	return 0;
}


void BootM1()
{
	if (cbm64->LoadAppWithoutBasic("/home/puzzud/temp/Desktop/mule.2") != 0)
	{
		cout << "Failed to load Intro" << endl;
		return;
	}

	cout << "Loaded Intro" << endl;
	
	// Set jump watch for right before loading M from disk.
	watcher->SetJumpWatch(M1_A_ProcessSettingsOver, WCB_MP_CAST(CustomWatcher, M1_ProcessSettingsOver));

	if (ShouldPlayIntro)
	{
		cbm64->GetCpu()->SetPC(M1_A_JumpToInitializeIntro);
	}
	else
	{
		cbm64->GetCpu()->SetPC(M1_A_JumpToInitializeIntro);
		watcher->SetAddressWatch(M1_A_InitializeOver, WCB_MP_CAST(CustomWatcher, M1_InitializeOver));
	}

	CurrentModuleId = 0;
	ScreenId = 0;
	SubScreenId = 0;
}


void M1_SetInitialGameDifficulty(u8 gameDifficulty)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, M1_A_InitialGameDifficulty, gameDifficulty);
}


// NOTE: Not an m2 variable.
void M1_SetNumberOfHumanPlayers(u8 number)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, M1_A_NumberOfHumanPlayers, number);
}


void M1_SetFakeIntroUserSettings()
{
	M1_SetInitialGameDifficulty(2);

	int numberOfHumanPlayers = 1;
	M1_SetNumberOfHumanPlayers(numberOfHumanPlayers);

	int numberOfAiPlayers = 4 - numberOfHumanPlayers;

	int playerIndex = 0;
	for (; playerIndex < numberOfAiPlayers; ++playerIndex)
	{
		MX_SetPlayerInputType(playerIndex, 0xff); // Computer
		MX_SetPlayerSpecies(playerIndex, 0); // Should be all Mechtron?
	}

	// NOTE: Human players must be ascend from
	// index 3 to 0, for mule to run correctly.
	// Mainly it's for the AI routines.
	int humanPlayerCount = 0;
	for (; playerIndex < 4; ++playerIndex, ++humanPlayerCount)
	{
		// 9 is Joystick 2.
		// 8 is Joystick 1.
		MX_SetPlayerInputType(playerIndex, 9 - humanPlayerCount);

		// 1 is Gollumer.
		MX_SetPlayerSpecies(playerIndex, 1 + humanPlayerCount);
	}

	MX_SetPlayerColor(0, 5);
	MX_SetPlayerColor(1, 6);
	MX_SetPlayerColor(2, 8);
	MX_SetPlayerColor(3, 4);
}


void M1_ApplySettingsAndJumpToDataCopy()
{
	M1_SetFakeIntroUserSettings();

	// Need to change some values to those
	// right before loading (these setup during m1 confirmation screen?).
	auto vic = cbm64->GetVic();
	vic->Poke(0xD015, 0x00); // Disable all sprites.
	vic->Poke(0xD01C, 0x00); // Make all sprites single color.

	auto vicMemoryControl = vic->Peek(0xD018); // Set up VIC character memory.
	vicMemoryControl &= 0xf1;
	vicMemoryControl |= 10;
	vic->Poke(0xD018, vicMemoryControl);

	// Part where various data is processed and copied
	// right before loading and proceeding to M.
	cbm64->GetCpu()->SetPC(M1_A_CopyPlayerSpeciesGraphics);
}


void M1_LoadAndBootM2()
{
	if (DelayBeforeM2Load)
	{
		u16 rasterLineNumber = cbm64->GetVic()->Peek(0xD012) | ((cbm64->GetVic()->Peek(0xD011) & 0x80) << 8);
		if (rasterLineNumber > 256)
		{
			int static timer = 10000;
			while (--timer == 0)
			{
				goto proceedToM2;
			}
		}

		cbm64->GetCpu()->SetPC(0x653A-3);
		return;
	}

proceedToM2:
	if (cbm64->LoadAppWithoutBasic("/home/puzzud/temp/Desktop/m") != 0)
	{
		cout << "Failed to load M2" << endl;
		return;
	}

	cout << "Loaded M" << endl;

	cbm64->GetCpu()->SetPC(M2_A_JumpToGameInitialize);
	CurrentModuleId = 1;
	
	watcher->ClearJumpWatch(M1_A_ProcessSettingsOver);

	watcher->SetAddressWatch(M2_A_GameInitialize, WCB_MP_CAST(CustomWatcher, M2_GameInitialize));
	watcher->SetAddressWatch(M2_A_DoRound, WCB_MP_CAST(CustomWatcher, M2_DoRound));
	watcher->SetAddressWatch(M2_A_DoLandGrant, WCB_MP_CAST(CustomWatcher, M2_DoLandGrant));
	watcher->SetAddressWatch(M2_A_DoPlayerLandAuction, WCB_MP_CAST(CustomWatcher, M2_DoPlayerLandAuction));
	watcher->SetAddressWatch(M2_A_DoRandomLandAuctions, WCB_MP_CAST(CustomWatcher, M2_DoRandomLandAuctions));
	watcher->SetAddressWatch(M2_A_DoDevelopmentTurn, WCB_MP_CAST(CustomWatcher, M2_DoDevelopmentTurn));
	watcher->SetAddressWatch(M2_A_DoRoundEvent, WCB_MP_CAST(CustomWatcher, M2_DoRoundEvent));
	watcher->SetAddressWatch(M2_A_DoShipArrive, WCB_MP_CAST(CustomWatcher, M2_DoShipArrive));
	watcher->SetAddressWatch(M2_A_DoShipLeave, WCB_MP_CAST(CustomWatcher, M2_DoShipLeave));
	watcher->SetAddressWatch(M2_A_EndRoundEvent, WCB_MP_CAST(CustomWatcher, M2_EndRoundEvent));
	watcher->SetAddressWatch(M2_A_DoGoodAuction, WCB_MP_CAST(CustomWatcher, M2_DoGoodAuction));
	watcher->SetAddressWatch(M2_A_AuctionTrade, WCB_MP_CAST(CustomWatcher, M2_AuctionTrade));
	watcher->SetAddressWatch(M2_A_CalculateScoresDoStatusScreen, WCB_MP_CAST(CustomWatcher, M2_CalculateScoresDoStatusScreen));

	//watcher->SetJumpWatch(M2_A_AddMessage);

	auto vic = cbm64->GetVic();
	vic->Poke(0xD011, vic->Peek(0xD011) & ~0x10);
}

void MX_SetPlayerInputType(unsigned int playerIndex, u8 playerInputType)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_A_PlayerInputType + playerIndex, playerInputType);
}


void MX_SetPlayerColor(unsigned int playerIndex, u8 playerColor)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_A_PlayerColor + playerIndex, playerColor);
}


void MX_SetPlayerSpecies(unsigned int playerIndex, u8 playerSpecies)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_A_PlayerSpecies + playerIndex, playerSpecies);
}
