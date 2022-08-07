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

typedef enum _eWatcherJumpType{
	eWatcherJump = 0,
	eWatcherJumpRoutine,
	eWatcherJumpInterrupt
}eWatcherJumpType;

class CWatcher{
private:
	// NOTE: Address watches limited to instruction fetch addresses.
	unsigned int mAddressWatches[0xFFFF];
	unsigned int mJumpWatches[0xFFFF];
	unsigned int mReadWatches[0xFFFF];
	unsigned int mWriteWatches[0xFFFF];
protected:
public:
	CWatcher();
	~CWatcher();

	void SetAddressWatch(u16 address);
	void ClearAddressWatch(u16 address);
	bool CheckAddressWatch(u16 address);
	virtual void ReportAddressWatch(u16 address) = 0;

	void SetJumpWatch(u16 address);
	void ClearJumpWatch(u16 address);
	bool CheckJumpWatch(u16 address);
	virtual void ReportJumpWatch(u16 address, eWatcherJumpType jumpType) = 0;

	void SetReadWatch(u16 address);
	void ClearReadWatch(u16 address);
	bool CheckReadWatch(u16 address);
	virtual void ReportReadWatch(u16 address) = 0;

	void SetWriteWatch(u16 address);
	void ClearWriteWatch(u16 address);
	bool CheckWriteWatch(u16 address);
	virtual void ReportWriteWatch(u16 address) = 0;
};

#endif //WATCHER_H
