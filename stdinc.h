
#pragma once

class AddressMgr;

extern AddressMgr g_addresses;

#define __MAJOR_REV__ 1
#define __MINOR_REV__ 0

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

#include "opcode.h"
#include "pattern.h"
#include "patch.h"
#include "enums.h"
#include "config.h"
#include "types.h"
#include "game.h"

#include "AddressMgr.h"

#include "Hooking.h"

#pragma region Game Includes

#include "profileSettingsMgr.h"

#pragma endregion

void main();
void unload();

extern HMODULE hCurrentModule;