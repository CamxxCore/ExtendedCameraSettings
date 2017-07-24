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

#include "../inc/enums.h"
#include "../inc/natives.h"

#include "../tinyxml2/tinyxml2.h"

#include "Utility/General.h"
#include "Utility/Logger.h"
#include "Utility/XMLHelper.h"
#include "Utility/Math.h"
#include "Utility/AddressMgr.h"
#include "Utility/keymap.h"
#include "Utility/config.h"
#include "Utility/patch.h"

#include "Utility/Pattern.h"
#include "Utility/Hooking.h"

#include "types.h"
#include "enums.h"
#include "strings.h"
#include "functions.h"
#include "presets.h"

#include "Game.h"

void scriptMain();

void scriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

void unload();
