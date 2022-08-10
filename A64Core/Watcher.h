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


typedef enum _eWatcherJumpType{
	eWatcherJump = 0,
	eWatcherJumpRoutine,
	eWatcherJumpInterrupt
}eWatcherJumpType;


class CWatcher{
	using WatchCallback = int (CWatcher::*)(u16 address);

private:
	// NOTE: Address watches limited to instruction fetch addresses.
	u8 mAddressWatches[WATCHER_ADDRESS_SIZE];
	WatchCallback mAddressCallbacks[WATCHER_ADDRESS_SIZE];

	u8 mJumpWatches[WATCHER_ADDRESS_SIZE];
	WatchCallback mJumpCallbacks[WATCHER_ADDRESS_SIZE];

	u8 mReadWatches[WATCHER_ADDRESS_SIZE];
	WatchCallback mReadCallbacks[WATCHER_ADDRESS_SIZE];

	u8 mWriteWatches[WATCHER_ADDRESS_SIZE];
	WatchCallback mWriteCallbacks[WATCHER_ADDRESS_SIZE];
protected:
public:
	CWatcher();
	~CWatcher();

	void SetAddressWatch(u16 address, WatchCallback callback = NULL);
	void ClearAddressWatch(u16 address);
	bool CheckAddressWatch(u16 address);
	virtual int GeneralReportAddressWatch(u16 address);

	void SetJumpWatch(u16 address, WatchCallback callback = NULL);
	void ClearJumpWatch(u16 address);
	bool CheckJumpWatch(u16 address, eWatcherJumpType jumpType);
	virtual int GeneralReportJumpWatch(u16 address, eWatcherJumpType jumpType);

	void SetReadWatch(u16 address, WatchCallback callback = NULL);
	void ClearReadWatch(u16 address);
	bool CheckReadWatch(u16 address);
	virtual int GeneralReportReadWatch(u16 address);

	void SetWriteWatch(u16 address, WatchCallback callback = NULL);
	void ClearWriteWatch(u16 address);
	bool CheckWriteWatch(u16 address);
	virtual int GeneralReportWriteWatch(u16 address);
};

#endif //WATCHER_H
