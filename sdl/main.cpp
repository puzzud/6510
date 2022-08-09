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

void SetInitialGameDifficulty(u8 gameDifficulty);
void SetPlayerInputType(unsigned int playerIndex, u8 playerInputType);
void SetPlayerColor(unsigned int playerIndex, u8 playerColor);
void SetPlayerSpecies(unsigned int playerIndex, u8 playerSpecies);
void SetFakeIntroUserSettings();


class CustomWatcher;


CBM64Main* cbm64 = NULL;
CustomWatcher* watcher = NULL;


class CustomWatcher : public CWatcher
{
    public:
	protected:
	
	virtual void ReportAddressWatch(u16 address)
	{
		if (address == 0x972B) // GameInitialize
		{
			cout << "m2 GameInitialize" << endl;
			return;
		}
		else if (address == 0x9771) //DoRound
		{
			cout << "DoRound" << endl;
			return;
		}
		else if (address == 0x55B3) // J_55B3 initial game initialization over
		//else if (address == 0x547F)
		{
			SetFakeIntroUserSettings();

			// Need to change some values to those
			// right before loading (these setup during m1 confirmation screen?).
			cbm64->GetVic()->Poke(0xD01C, 0x00);

			// Part where various data is processed and copied
			// right before loading and proceeding to M.
			//CopyPlayerSpeciesGraphics:            ;       [6427]
			cbm64->GetCpu()->SetPC(0x6427);
			//cbm64->GetCpu()->SetPC(0x6290);

			return;
		}

		cout << "Watcher Address: " << std::hex << int(address) << std::dec << endl;
	};

	virtual void ReportJumpWatch(u16 address, eWatcherJumpType jumpType)
	{
		cout << "Watcher Jump: " << std::hex << int(address) << std::dec << endl;

		if (address == 0x655B)
		{
			if (cbm64->LoadAppWithoutBasic("/home/puzzud/temp/Desktop/m") == 0)
			{
				cout << "Loaded M" << endl;
				cbm64->GetCpu()->SetPC(0x4000);
				
				ClearJumpWatch(0x655B);

				SetAddressWatch(0x972B); // GameInitialize
				SetAddressWatch(0x9771); // DoRound

				SetReadWatch(0xE509+0);
				SetReadWatch(0xE509+1);
				SetReadWatch(0xE509+2);
				SetReadWatch(0xE509+3);

				SetWriteWatch(0x48);
				SetWriteWatch(0xDF12);
			}
		}
	};

	virtual void ReportReadWatch(u16 address)
	{
		if (address >= 0xE509 && address < 0xE509+4)
		{
			auto bus = CBus::GetInstance();
			u8 planeteerSpecies = bus->Peek(address);

			cout << "PlaneteerSpecies Read: " << int(planeteerSpecies) << endl;

			return;
		}

		cout << "Watcher Read: " << std::hex << int(address) << std::dec << endl;
	}

	virtual void ReportWriteWatch(u16 address)
	{
		if (address == 0x48)
		{
			auto bus = CBus::GetInstance();
			u8 roundNumber = bus->Peek(address);

			cout << "RoundNumber" << ": " << int(roundNumber) << endl;

			return;
		}else if (address == 0xDF12)
		{
			cout  << std::hex;
			cout << "Watcher Write: " << int(address);
			cout << " before PC: " << int(cbm64->GetCpu()->GetPC()) << endl;
			cout << std::dec;
			return;
		}

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

	if (cbm64->LoadAppWithoutBasic("/home/puzzud/temp/Desktop/mule.2") != 0)
	{
		cout << "Failed to load Intro" << endl;
		return -1;
	}

	cout << "Loaded Intro" << endl;
	
	watcher->SetJumpWatch(0x655B); // Right before loading M from disk.

	const bool shouldPlayIntro = false;
	if (shouldPlayIntro)
	{
		cbm64->GetCpu()->SetPC(0x4000);
	}
	else
	{
		cbm64->GetCpu()->SetPC(0x4000);

		watcher->SetAddressWatch(0x55B3); // J_55B3 // Address just enough to run initial setup.
		//watcher->SetAddressWatch(0x547F);
	}

	MainLoop();

	hwController.Shutdown();
	hwScreen.Shutdown();

	delete watcher;
	watcher = NULL;

	return 0;
}


void SetInitialGameDifficulty(u8 gameDifficulty)
{
	auto bus = CBus::GetInstance();

	//InitialGameDifficulty    =       $E504
	bus->PokeDevice(eBusRam, 0xe504, gameDifficulty);
}


void SetPlayerInputType(unsigned int playerIndex, u8 playerInputType)
{
	auto bus = CBus::GetInstance();

	//PlayerInputType          =       $E500
	bus->PokeDevice(eBusRam, 0xe500 + playerIndex, playerInputType);
}


void SetPlayerColor(unsigned int playerIndex, u8 playerColor)
{
	auto bus = CBus::GetInstance();

	//PlayerColor               =       $E505
	bus->PokeDevice(eBusRam, 0xe505 + playerIndex, playerColor);
}


void SetPlayerSpecies(unsigned int playerIndex, u8 playerSpecies)
{
	auto bus = CBus::GetInstance();

	//PlayerSpecies             =       $E509
	bus->PokeDevice(eBusRam, 0xe509 + playerIndex, playerSpecies);
}


// NOTE: Not an m2 variable.
void SetNumberOfHumanPlayers(u8 number)
{
	auto bus = CBus::GetInstance();

	//NumberOfHumanPlayers:           ;       [43FF]
	bus->PokeDevice(eBusRam, 0x43FF, number);
}


void SetFakeIntroUserSettings()
{
	SetInitialGameDifficulty(2);

	for (int i = 0; i < 4; ++i)
	{
		SetPlayerInputType(i, 0xff); // Computer
		SetPlayerSpecies(i, 0); // Should be all Mechtron?
	}

	SetPlayerInputType(0, 9); // Joystick 2.

	SetPlayerColor(0, 5);
	SetPlayerColor(1, 6);
	SetPlayerColor(2, 8);
	SetPlayerColor(3, 4);

	SetNumberOfHumanPlayers(1);
}
