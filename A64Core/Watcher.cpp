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
    memset(mAddressWatches, 0, WATCHER_ADDRESS_SIZE);
    memset(mJumpWatches, 0, WATCHER_ADDRESS_SIZE);
    memset(mReadWatches, 0, WATCHER_ADDRESS_SIZE);
    memset(mWriteWatches, 0, WATCHER_ADDRESS_SIZE);

    for (int i = 0; i < WATCHER_ADDRESS_SIZE; ++i){
        mAddressCallbacks[i] = NULL;
        mJumpCallbacks[i] = NULL;
        mReadCallbacks[i] = NULL;
        mWriteCallbacks[i] = NULL;
    }
}

CWatcher::~CWatcher(){
}


void CWatcher::SetAddressWatch(u16 address, WatchCallback callback){
    mAddressWatches[address] = 1;
    mAddressCallbacks[address] = callback;
}


void CWatcher::ClearAddressWatch(u16 address){
    mAddressWatches[address] = 0;
    mAddressCallbacks[address] = NULL;
}


bool CWatcher::CheckAddressWatch(u16 address){
    if (mAddressWatches[address] != 0){
        WatchCallback callback = mAddressCallbacks[address];
        if (callback != NULL){
            watchAddress = address;
            return (this->*callback)() != 0;
        }

        return GeneralReportAddressWatch(address) != 0;
    }

    return false;
}


int CWatcher::GeneralReportAddressWatch(u16 address){
    return 0;
}


void CWatcher::SetJumpWatch(u16 address, WatchCallback callback){
    mJumpWatches[address] = 1;
    mJumpCallbacks[address] = callback;
}


void CWatcher::ClearJumpWatch(u16 address){
    mJumpWatches[address] = 0;
    mJumpCallbacks[address] = NULL;
}


bool CWatcher::CheckJumpWatch(u16 address, eWatcherJumpType jumpType){
    if (mJumpWatches[address] != 0){
        WatchCallback callback = mJumpCallbacks[address];
        if (callback != NULL){
            watchAddress = address;
            this->jumpType = jumpType;
            return (this->*callback)() != 0;
        }

        return GeneralReportJumpWatch(address, jumpType) != 0;
    }

    return false;
}


int CWatcher::GeneralReportJumpWatch(u16 address, eWatcherJumpType jumpType){
    return 0;
}


void CWatcher::SetReadWatch(u16 address, WatchCallback callback){
    mReadWatches[address] = 1;
    mReadCallbacks[address] = callback;
}


void CWatcher::ClearReadWatch(u16 address){
    mReadWatches[address] = 0;
    mReadCallbacks[address] = NULL;
}


bool CWatcher::CheckReadWatch(u16 address, u8 value){
    if (mReadWatches[address] != 0){
        WatchCallback callback = mReadCallbacks[address];
        if (callback != NULL){
            watchAddress = address;
            this->value = value;
            return (this->*callback)() != 0;
        }

        return GeneralReportReadWatch(address) != 0;
    }

    return false;
}


int CWatcher::GeneralReportReadWatch(u16 address){
    return 0;
}


void CWatcher::SetWriteWatch(u16 address, WatchCallback callback){
    mWriteWatches[address] = 1;
    mWriteCallbacks[address] = callback;
}


void CWatcher::ClearWriteWatch(u16 address){
    mWriteWatches[address] = 0;
    mWriteCallbacks[address] = NULL;
}


bool CWatcher::CheckWriteWatch(u16 address, u8 value){
    if (mWriteWatches[address] != 0){
        WatchCallback callback = mWriteCallbacks[address];
        if (callback != NULL){
            watchAddress = address;
            this->value = value;
            return (this->*callback)() != 0;
        }

        return GeneralReportWriteWatch(address) != 0;
    }

    return false;
}


int CWatcher::GeneralReportWriteWatch(u16 address){
    return 0;
}
