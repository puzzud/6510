/*
 *  Watcher.h
 *  A64Emulator
 *
 *  Created by Bruno Keymolen on 10/07/08.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef WATCHER_H
#define WATCHER_H

#include "Common.h"


#define WATCHER_ADDRESS_SIZE                0x10000

#define WCB_MP_CAST(classname,methodname) ((classname::WatchCallback)&classname::methodname)

typedef enum _eWatcherJumpType{
	eWatcherJump = 0,
	eWatcherJumpRoutine,
	eWatcherJumpInterrupt
}eWatcherJumpType;

class CWatcher{
public:
	using WatchCallback = int (CWatcher::*)();

	typedef struct _SWatchSetting{
		bool enabled;
		bool isRange;
		u16 rangeOffset;
		WatchCallback callback;
	}SWatchSetting;

private:
	// NOTE: Address watches limited to instruction fetch addresses.
	SWatchSetting mAddressWatches[WATCHER_ADDRESS_SIZE];
	SWatchSetting mJumpWatches[WATCHER_ADDRESS_SIZE];
	SWatchSetting mReadWatches[WATCHER_ADDRESS_SIZE];
	SWatchSetting mWriteWatches[WATCHER_ADDRESS_SIZE];

protected:
	void ClearWatchSetting(SWatchSetting& watchSetting);

	// Address involved in most recent triggered watch.
	u16 watchAddress;
	
	// Offset if watch address was specified as a range.
	u16 watchRangeOffset;
	
	// Value involved in most recent triggered watch (read or set).
	u8 watchValue;
	
	// Type of jump of the most recent jump watch triggered.
	eWatcherJumpType watchJumpType;

	// Address that RTS will redirect,
	// valid for most recent eWatcherJumpRoutine watch triggered.
	u16 jumpWatchReturnAddress;

public:
	CWatcher();
	~CWatcher();

	void SetAddressWatch(u16 address, WatchCallback callback = NULL);
	void SetAddressWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback = NULL);
	void ClearAddressWatch(u16 address);
	void ClearAddressWatchRange(u16 address_range_start, u16 address_range_end);
	bool CheckAddressWatch(u16 address);
	virtual int GeneralReportAddressWatch(u16 address);

	void SetJumpWatch(u16 address, WatchCallback callback = NULL);
	void SetJumpWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback = NULL);
	void ClearJumpWatch(u16 address);
	void ClearJumpWatchRange(u16 address_range_start, u16 address_range_end);
	bool CheckJumpWatch(u16 address, eWatcherJumpType jumpType, u16* returnAddress = NULL);
	virtual int GeneralReportJumpWatch(u16 address, eWatcherJumpType jumpType);

	void SetReadWatch(u16 address, WatchCallback callback = NULL);
	void SetReadWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback = NULL);
	void ClearReadWatch(u16 address);
	void ClearReadWatchRange(u16 address_range_start, u16 address_range_end);
	bool CheckReadWatch(u16 address, u8 value);
	u16 GetReadWatchRangeOffset(u16 address);
	virtual int GeneralReportReadWatch(u16 address);

	void SetWriteWatch(u16 address, WatchCallback callback = NULL);
	void SetWriteWatchRange(u16 address_range_start, u16 address_range_end, WatchCallback callback = NULL);
	void ClearWriteWatch(u16 address);
	void ClearWriteWatchRange(u16 address_range_start, u16 address_range_end);
	bool CheckWriteWatch(u16 address, u8 value);
	u16 GetWriteWatchRangeOffset(u16 address);
	virtual int GeneralReportWriteWatch(u16 address);
};

#endif //WATCHER_H
