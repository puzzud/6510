/*
 *  Common.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 04/06/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */



#ifndef COMMON_H
#define COMMON_H

#include <cstdint>
#include <string.h>
#include <iostream>
#include <ostream>
#include <iomanip>
#include "bkegen.h"

using namespace std;

class CDebugOutStream : public std::ostream {
public:
    CDebugOutStream() : std::ostream(nullptr) {
#ifdef DEBUG
        rdbuf(cout.rdbuf());
#endif
    }
};

extern CDebugOutStream debug_out;

#endif