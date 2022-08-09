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


CBM64Main* cbm64 = NULL;


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

	if (argc > 1){
		if (cbm64->LoadAppWithoutBasic(argv[1]) == 0){
			if (argc > 2){
				cbm64->GetCpu()->SetPC(std::strtol(argv[2], NULL, 16));
			}
		}
	}

	MainLoop();

	hwController.Shutdown();
	hwScreen.Shutdown();

	std::cout << "ended..." << std::endl;

	return 0;
}
