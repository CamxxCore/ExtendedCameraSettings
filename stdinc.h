#pragma once

class AddressMgr;

extern AddressMgr g_addresses;


#define __MAJOR_REV__ 1.0
#define __MINOR_REV__ .03

#define APP_VERSION (__MAJOR_REV__ + __MINOR_REV__)

#define WriteInt(a,b)(*(int*)a = (int)b)

#define WriteFloat(a,b)(*(float*)a = (float)b)

#define WriteBoolean(a,b)(*(bool*)a = (bool)b)

#include <windows.h>

#include <map>
#include <vector>
#include <mutex>
#include <fstream>

#include "resource.h"

#include "inc/enums.h"
#include "inc/types.h"
#include "inc/natives.h"
#include "inc/main.h"

#include "logger.h"
#include "utility.h"
#include "opcode.h"
#include "pattern.h"
#include "patch.h"
#include "enums.h"

#include "config.h"
#include "types.h"
#include "game.h"

#include "AddressMgr.h"

#include "Hooking.h"

inline void printToScreen(const char * fmt, ...)
{
	char inBuf[0x100];

	va_list va;

	va_start(va, fmt);

	vsprintf_s(inBuf, fmt, va);

	va_end(va);

	char buffer[0x100];

	sprintf_s(buffer, "~y~ExtendedCameraSettings~w~\n%s", inBuf);

	notifyAboveMap(buffer);
}

void main();

void unload();

extern HMODULE hCurrentModule;
