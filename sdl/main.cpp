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

#define M1_JumpToInitializeIntro               0x4000
#define M1_InitializeOver                      0x55b3
#define M1_CopyPlayerSpeciesGraphics           0x6427
#define M1_ProcessSettingsOver                 0x655b

#define M1_NumberOfHumanPlayers                0x43ff
#define M1_InitialGameDifficulty               0xe504

#define M2_JumpToGameInitialize                0x4000
#define M2_GameInitialize                      0x972b
#define M2_DoRound                             0x9771

#define M2_RoundNumber                         0x0048
#define M2_UnmappedIoSpaceStart                0xdf12

#define MX_PlayerInputType                     0xe500
#define MX_PlayerColor                         0xe505
#define MX_PlayerSpecies                       0xe509

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
bool ShouldPlayIntro = false;
bool DelayBeforeM2Load = false;

class CustomWatcher : public CWatcher
{
    public:
	protected:
	
	virtual int ReportAddressWatch(u16 address)
	{
		if (address == M1_InitializeOver)
		{
			M1_ApplySettingsAndJumpToDataCopy();

			return 1;
		}
		/*else if (address == M2_GameInitialize) // GameInitialize
		{
			cout << "GameInitialize" << endl;
			return;
		}
		else if (address == M2_DoRound) //DoRound
		{
			cout << "DoRound" << endl;
			return;
		}
		*/

		cout << "Watcher Address: " << std::hex << int(address) << std::dec << endl;

		return 0;
	};

	virtual int ReportJumpWatch(u16 address, eWatcherJumpType jumpType)
	{
		if (address == M1_ProcessSettingsOver)
		{
			M1_LoadAndBootM2();
			return 1;
		}

		cout << "Watcher Jump: " << std::hex << int(address) << std::dec << endl;

		return 0;
	};

	virtual void ReportReadWatch(u16 address)
	{
		cout << "Watcher Read: " << std::hex << int(address) << std::dec << endl;
	}

	virtual void ReportWriteWatch(u16 address)
	{
		/*if (address == M2_RoundNumber)
		{
			auto bus = CBus::GetInstance();
			u8 roundNumber = bus->Peek(address);

			cout << "RoundNumber" << ": " << int(roundNumber) << endl;

			return;
		}else if (address == M2_UnmappedIoSpaceStart)
		{
			cout  << std::hex;
			cout << "Watcher Write: " << int(address);
			cout << " before PC: " << int(cbm64->GetCpu()->GetPC()) << endl;
			cout << std::dec;
			return;
		}*/

		cout << "Watcher Write: " << std::hex << int(address) << std::dec << endl;
	}

    private:
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
	watcher->SetJumpWatch(M1_ProcessSettingsOver);

	if (ShouldPlayIntro)
	{
		cbm64->GetCpu()->SetPC(M1_JumpToInitializeIntro);
	}
	else
	{
		cbm64->GetCpu()->SetPC(M1_JumpToInitializeIntro);
		watcher->SetAddressWatch(M1_InitializeOver);
	}

	CurrentModuleId = 0;
}


void M1_SetInitialGameDifficulty(u8 gameDifficulty)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, M1_InitialGameDifficulty, gameDifficulty);
}


// NOTE: Not an m2 variable.
void M1_SetNumberOfHumanPlayers(u8 number)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, M1_NumberOfHumanPlayers, number);
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
	cbm64->GetCpu()->SetPC(M1_CopyPlayerSpeciesGraphics);
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

	cbm64->GetCpu()->SetPC(M2_JumpToGameInitialize);
	CurrentModuleId = 1;
	
	watcher->ClearJumpWatch(M1_ProcessSettingsOver);

	//watcher->SetAddressWatch(M2_GameInitialize);
	//watcher->SetAddressWatch(M2_DoRound);

	//SetWriteWatch(M2_RoundNumber);
	//SetWriteWatch(M2_UnmappedIoSpaceStart);

	auto vic = cbm64->GetVic();
	vic->Poke(0xD011, vic->Peek(0xD011) & ~0x10);
}

void MX_SetPlayerInputType(unsigned int playerIndex, u8 playerInputType)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_PlayerInputType + playerIndex, playerInputType);
}


void MX_SetPlayerColor(unsigned int playerIndex, u8 playerColor)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_PlayerColor + playerIndex, playerColor);
}


void MX_SetPlayerSpecies(unsigned int playerIndex, u8 playerSpecies)
{
	auto bus = CBus::GetInstance();
	bus->PokeDevice(eBusRam, MX_PlayerSpecies + playerIndex, playerSpecies);
}
