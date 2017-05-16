#pragma once

class AddressMgr;

extern AddressMgr g_addresses;

#define __MAJOR_REV__ 1.1
#define __MINOR_REV__ .02

#define APP_VERSION (__MAJOR_REV__ + __MINOR_REV__)

#define WriteInt(a,b)(*(int*)a = b)

#define WriteUInt(a,b)(*(unsigned int*)a = b)

#define WriteFloat(a,b)(*(float*)a = b)

#define WriteDouble(a,b)(*(double*)a = b)

#define WriteBool(a,b)(*(bool*)a = b)

#define MAX_STRING 256

#include <windows.h>

#include <map>
#include <vector>
#include <mutex>
#include <fstream>
#include <algorithm>
#include <string>

#include "resource.h"

#include "opcode.h"
#include "pattern.h"
#include "patch.h"

#include "enums.h"
#include "keymap.h"
#include "strings.h"
#include "config.h"
#include "game.h"
#include "functions.h"
#include "types.h"

#include "tinyxml2.h"
#include "XMLHelper.h"

#include "Utility.h"

#include "Math.h"

#include "AddressMgr.h"

#include "Logger.h"

#include "Hooking.h"

inline void printToScreen(const char * format, ...)
{
	char inBuf[MAX_STRING];

	va_list va;

	va_start(va, format);

	vsprintf_s(inBuf, format, va);

	va_end(va);

	char szText[MAX_STRING];

	auto prefix = Utility::GetModuleName(Utility::GetActiveModule());

	sprintf_s(szText, "~y~%s~w~\n%s", prefix.c_str(), inBuf);

	notifyAboveMap(szText);
}

void mainInit(HMODULE hModule);

void scriptMain();

void scriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

void unload();
