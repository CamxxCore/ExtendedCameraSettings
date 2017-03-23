
#pragma once

#include <windows.h>
#include <Psapi.h>
#include <inttypes.h>

#include <cstdio>
#include <map>
#include <sstream>
#include <assert.h>
#include <vector>
#include <mutex>
#include <iterator>

#include <math.h>

#include "inc/enums.h"
#include "inc/types.h"
#include "inc/natives.h"
#include "inc/main.h"

#include "opcode.h"
#include "pattern.h"
#include "patch.h"
#include "enums.h"
#include "config.h"

#include "AddressMgr.h"

extern AddressMgr g_addresses;

#include "types.h"

#include "Hooking.h"

#include "game.h"

#include "profileSettingsMgr.h"

void main();

void unload();