/*
 *  Watcher.cpp
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "Watcher.h"

CWatcher::CWatcher(){
    memset(mJumpWatches, 0, sizeof(unsigned int) * 0xFFFF);
}

CWatcher::~CWatcher(){
}

void CWatcher::SetJumpWatch(u16 address){
    mJumpWatches[address] = true;
}

void CWatcher::ClearJumpWatch(u16 address){
    mJumpWatches[address] = false;
}

bool CWatcher::CheckJumpWatch(u16 address, eWatcherJumpType jumpType){
    if (mJumpWatches[address]){
        ReportJumpWatch(address, jumpType);
        return true;
    }

    return false;
}
