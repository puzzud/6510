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
	unsigned int mJumpWatches[0xFFFF];
protected:
	virtual void ReportJumpWatch(u16 address, eWatcherJumpType jumpType) = 0;
public:
	CWatcher();
	~CWatcher();
	void SetJumpWatch(u16 address);
	void ClearJumpWatch(u16 address);
	bool CheckJumpWatch(u16 address, eWatcherJumpType jumpType);
};

#endif //WATCHER_H
