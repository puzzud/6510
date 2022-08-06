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
    memset(mReadWatches, 0, sizeof(unsigned int) * 0xFFFF);
    memset(mWriteWatches, 0, sizeof(unsigned int) * 0xFFFF);
}

CWatcher::~CWatcher(){
}

void CWatcher::SetJumpWatch(u16 address){
    mJumpWatches[address] = 1;
}

void CWatcher::ClearJumpWatch(u16 address){
    mJumpWatches[address] = 0;
}

bool CWatcher::CheckJumpWatch(u16 address){
    return mJumpWatches[address] != 0;
}

void CWatcher::SetReadWatch(u16 address){
    mReadWatches[address] = 1;
}

void CWatcher::ClearReadWatch(u16 address){
    mReadWatches[address] = 0;
}

bool CWatcher::CheckReadWatch(u16 address){
    return mReadWatches[address] != 0;
}

void CWatcher::SetWriteWatch(u16 address){
    mWriteWatches[address] = 1;
}

void CWatcher::ClearWriteWatch(u16 address){
    mWriteWatches[address] = 0;
}

bool CWatcher::CheckWriteWatch(u16 address){
    return mWriteWatches[address] != 0;
}
