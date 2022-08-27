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
    
    ClearAddressWatchRange(0x0000, 0xFFFF);
    ClearJumpWatchRange(0x0000, 0xFFFF);
    ClearReadWatchRange(0x0000, 0xFFFF);
    ClearWriteWatchRange(0x0000, 0xFFFF);
}

CWatcher::~CWatcher(){
}


void CWatcher::ClearWatchSetting(SWatchSetting& watchSetting){
    watchSetting.enabled = false;
    watchSetting.isRange = false;
    watchSetting.rangeOffset = 0;
    watchSetting.callback = NULL;
}


void CWatcher::SetAddressWatch(u16 address, WatchCallback callback){
    ClearAddressWatch(address);

    SWatchSetting& watchSetting = mAddressWatches[address];
    watchSetting.enabled = true;
    watchSetting.callback = callback;
}


void CWatcher::SetAddressWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback){
    u16 rangeOffset = 0;
    for (uint32_t address = address_range_start; address <= address_range_end; ++address, ++rangeOffset){
        SetAddressWatch(address, callback);

        SWatchSetting& watchSetting = mAddressWatches[address];
        watchSetting.isRange = true;
        watchSetting.rangeOffset = rangeOffset;
    }
}


void CWatcher::ClearAddressWatch(u16 address){
    ClearWatchSetting(mAddressWatches[address]);
}


void CWatcher::ClearAddressWatchRange(u16 address_range_start, u16 address_range_end){
    for (uint32_t address = address_range_start; address <= address_range_end; ++address){
        ClearAddressWatch(address);
    }
}


bool CWatcher::CheckAddressWatch(u16 address){
    SWatchSetting& watchSetting = mAddressWatches[address];
    watchAddress = address;
    watchRangeOffset = watchSetting.rangeOffset;

    if (watchSetting.enabled){
        WatchCallback callback = watchSetting.callback;
        if (callback != NULL){
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
    ClearJumpWatch(address);

    SWatchSetting& watchSetting = mJumpWatches[address];
    watchSetting.enabled = true;
    watchSetting.callback = callback;
}


void CWatcher::SetJumpWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback){
    u16 rangeOffset = 0;
    for (uint32_t address = address_range_start; address <= address_range_end; ++address, ++rangeOffset){
        SetJumpWatch(address, callback);

        SWatchSetting& watchSetting = mJumpWatches[address];
        watchSetting.isRange = true;
        watchSetting.rangeOffset = rangeOffset;
    }
}


void CWatcher::ClearJumpWatch(u16 address){
    ClearWatchSetting(mJumpWatches[address]);
}


void CWatcher::ClearJumpWatchRange(u16 address_range_start, u16 address_range_end){
    for (uint32_t address = address_range_start; address <= address_range_end; ++address){
        ClearJumpWatch(address);
    }
}


bool CWatcher::CheckJumpWatch(u16 address, eWatcherJumpType jumpType, u16* returnAddress){
    SWatchSetting& watchSetting = mJumpWatches[address];
    watchAddress = address;
    watchJumpType = jumpType;
    watchRangeOffset = watchSetting.rangeOffset;

    if (returnAddress != NULL){
        jumpWatchReturnAddress = *returnAddress;
    }

    if (watchSetting.enabled){
        WatchCallback callback = watchSetting.callback;
        if (callback != NULL){
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
    ClearReadWatch(address);

    SWatchSetting& watchSetting = mReadWatches[address];
    watchSetting.enabled = true;
    watchSetting.callback = callback;
}


void CWatcher::SetReadWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback){
    u16 rangeOffset = 0;
    for (uint32_t address = address_range_start; address <= address_range_end; ++address, ++rangeOffset){
        SetReadWatch(address, callback);

        SWatchSetting& watchSetting = mReadWatches[address];
        watchSetting.isRange = true;
        watchSetting.rangeOffset = rangeOffset;
    }
}


void CWatcher::ClearReadWatch(u16 address){
    ClearWatchSetting(mReadWatches[address]);
}


void CWatcher::ClearReadWatchRange(u16 address_range_start, u16 address_range_end){
    for (uint32_t address = address_range_start; address <= address_range_end; ++address){
        ClearReadWatch(address);
    }
}


bool CWatcher::CheckReadWatch(u16 address, u8 value){
    SWatchSetting& watchSetting = mReadWatches[address];
    watchAddress = address;
    watchValue = value;
    watchRangeOffset = watchSetting.rangeOffset;

    if (watchSetting.enabled){
        WatchCallback callback = watchSetting.callback;
        if (callback != NULL){
            return (this->*callback)() != 0;
        }

        return GeneralReportReadWatch(address) != 0;
    }

    return false;
}


u16 CWatcher::GetReadWatchRangeOffset(u16 address){
    return mReadWatches[address].rangeOffset;
}


int CWatcher::GeneralReportReadWatch(u16 address){
    return 0;
}


void CWatcher::SetWriteWatch(u16 address, WatchCallback callback){
    ClearWriteWatch(address);

    SWatchSetting& watchSetting = mWriteWatches[address];
    watchSetting.enabled = true;
    watchSetting.callback = callback;
}


void CWatcher::SetWriteWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback){
    u16 rangeOffset = 0;
    for (uint32_t address = address_range_start; address <= address_range_end; ++address, ++rangeOffset){
        SetWriteWatch(address, callback);

        SWatchSetting& watchSetting = mWriteWatches[address];
        watchSetting.isRange = true;
        watchSetting.rangeOffset = rangeOffset;
    }
}


void CWatcher::ClearWriteWatch(u16 address){
    ClearWatchSetting(mWriteWatches[address]);
}


void CWatcher::ClearWriteWatchRange(u16 address_range_start, u16 address_range_end){
    for (uint32_t address = address_range_start; address <= address_range_end; ++address){
        ClearWriteWatch(address);
    }
}


bool CWatcher::CheckWriteWatch(u16 address, u8 value){
    SWatchSetting& watchSetting = mWriteWatches[address];
    watchAddress = address;
    watchValue = value;
    watchRangeOffset = watchSetting.rangeOffset;

    if (watchSetting.enabled){
        WatchCallback callback = watchSetting.callback;
        if (callback != NULL){
            return (this->*callback)() != 0;
        }

        return GeneralReportWriteWatch(address) != 0;
    }

    return false;
}


u16 CWatcher::GetWriteWatchRangeOffset(u16 address){
    return mWriteWatches[address].rangeOffset;
}



int CWatcher::GeneralReportWriteWatch(u16 address){
    return 0;
}
