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
			cout << "GameInitialize" << endl;
			return;
		}
		else if (address == 0x9771) //DoRound
		{
			cout << "DoRound" << endl;
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

				SetAddressWatch(0x972B);
				SetAddressWatch(0x9771);

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

	watcher = new CustomWatcher();
	cbm64->SetWatcher(watcher);

	if (argc > 1){
		if (cbm64->LoadAppWithoutBasic(argv[1]) == 0){
			watcher->SetJumpWatch(0x655B);
		
			if (argc > 2){
				cbm64->GetCpu()->SetPC(std::strtol(argv[2], NULL, 16));
			}
		}
	}

	MainLoop();

	hwController.Shutdown();
	hwScreen.Shutdown();

	delete watcher;
	watcher = NULL;

	std::cout << "ended..." << std::endl;

	return 0;
}
