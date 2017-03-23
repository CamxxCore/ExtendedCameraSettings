
#include "stdinc.h"

typedef int offset_t;

AddressMgr g_addresses;

bytepatch_t g_cinematicCameraEnterWaterPatch1, g_cinematicCameraEnterWaterPatch2;

eGameState *g_gameState;

CConfig g_scriptconfig = CConfig("POVCameraUnlocker.ini");

rage::pgCollection<camMetadataRef*> * g_metadataCollection;

rage::pgCollection<PauseMenuInstance> activeMenuArray;

typedef const char *(*GetGlobalTextEntry_t)(const char * text, unsigned int hashName);

typedef void(*SetPauseMenuPreference_t)(long long settingIndex, int value, unsigned int unk);

typedef bool(*SetMenuSlot_t)(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bIsSlotUpdate);

CallHook<GetGlobalTextEntry_t> * g_getGxtEntryFn;

CallHook<SetPauseMenuPreference_t> * g_setPauseMenuPreferenceFn;

CallHook<SetMenuSlot_t> * g_createSliderItemFn;

CallHook<SetMenuSlot_t> * g_createToggleItemFn;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

bool firstPersonUseReticle;

int firstPersonVehicleFovScale = 5;

bool followVehicleHighSpeedShake = false;

int g_disableEnterWaterCameraSwap;

int numPauseMenuItems = 5;

void addOffsetsForGameVersion(int gameVer)
{
	auto addresses = g_addresses.getOrCreate("general");

	addresses->insert("getProfileSetting", 0x140194ED0);

	addresses->insert("getProfileSettingIndex", 0x14018B5A0);

	addresses->insert("registerProfileSetting", 0x1401E55C8);
	
	#pragma region firstPersonCamera

	auto offsets = g_addresses.getOrCreate("firstPersonCamera");

	offsets->insert("fov", 36);

	offsets->insert("minPitch", 84);

	offsets->insert("maxPitch", 88);

	offsets->insert("altMinYaw", gameVer < v1_0_505_2_STEAM ? 696 : gameVer < v1_0_877_1_STEAM ? 712 : 760);

	offsets->insert("altMaxYaw", gameVer < v1_0_505_2_STEAM ? 700 : gameVer < v1_0_877_1_STEAM ? 716 : 764);

	offsets->insert("altMinPitch", gameVer < v1_0_505_2_STEAM ? 704 : gameVer < v1_0_877_1_STEAM ? 720 : 768);

	offsets->insert("altMaxPitch", gameVer < v1_0_505_2_STEAM ? 708 : gameVer < v1_0_877_1_STEAM ? 724 : 772);

	offsets->insert("relativeOffset", gameVer < v1_0_505_2_STEAM ? 68 : 64);

	offsets->insert("useReticule", 114);

#pragma endregion

	#pragma region cinematicMountedCamera

	offsets = g_addresses.getOrCreate("cinematicMountedCamera");

	offsets->insert("fov", gameVer < v1_0_877_1_STEAM ? 80 : 84);

	offsets->insert("minPitch", gameVer < v1_0_505_2_STEAM ? 808 : gameVer < v1_0_877_1_STEAM ? 824 : gameVer < v1_0_944_2_STEAM ? 872 : 888);

	offsets->insert("maxPitch", gameVer < v1_0_505_2_STEAM ? 812 : gameVer < v1_0_877_1_STEAM ? 828u : gameVer < v1_0_944_2_STEAM ? 876 : 892);

	offsets->insert("minPitchOverride", gameVer < v1_0_505_2_STEAM ? 776 : gameVer < v1_0_877_1_STEAM ? 792 : gameVer < v1_0_944_2_STEAM ? 840 : 856);

	offsets->insert("maxPitchOverride", gameVer < v1_0_505_2_STEAM ? 780 : gameVer < v1_0_877_1_STEAM ? 796 : gameVer < v1_0_944_2_STEAM ? 844 : 860);

	offsets->insert("minSpeedForCorrect", gameVer < v1_0_505_2_STEAM ? 680 : gameVer < v1_0_877_1_STEAM ? 696 : 744);

	offsets->insert("relativeOffset", gameVer < v1_0_877_1_STEAM ? 80 : 96);

#pragma endregion

#pragma region followVehicleCamera

	offsets = g_addresses.getOrCreate("followVehicleCamera");

	offsets->insert("minSpeedForShake", gameVer < v1_0_505_2_STEAM ? 1176 : 1192);

	offsets->insert("minSpeedForZoom", gameVer < v1_0_505_2_STEAM ? 1176 : 1192);

	offsets->insert("minSpeedForPopZoom", gameVer < v1_0_505_2_STEAM ? 1176 : 1192);

#pragma endregion
}

void patchFirstPersonShooterCameraMetadata(uintptr_t address)
{
	AddressPool addresses = (*g_addresses.get("firstPersonCamera"));

	*reinterpret_cast<float*>(address + addresses["fov"]) =
		g_scriptconfig.get<float>("OnFootCamera", "FOV", 45.0f);

	*reinterpret_cast<float*>(address + addresses["minPitch"]) =
		g_scriptconfig.get<float>("OnFootCamera", "MinPitch", -80.0f);

	*reinterpret_cast<float*>(address + addresses["maxPitch"]) =
		g_scriptconfig.get<float>("OnFootCamera", "MaxPitch", 80.0f);

	*reinterpret_cast<float*>(address + addresses["altMinPitch"]) =
		g_scriptconfig.get<float>("OnFootCamera", "AltMinPitch", -75.0f);

	*reinterpret_cast<float*>(address + addresses["altMaxPitch"]) =
		g_scriptconfig.get<float>("OnFootCamera", "AltMaxPitch", 20.0f);

	*reinterpret_cast<float*>(address + addresses["altMinYaw"]) =
		g_scriptconfig.get<float>("OnFootCamera", "AltMinYaw", -45.0f);

	*reinterpret_cast<float*>(address + addresses["altMaxYaw"]) =
		g_scriptconfig.get<float>("OnFootCamera", "AltMaxYaw", 45.0f);

	*reinterpret_cast<float*>(address + addresses["relativeOffset"]) =
		g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetX", 0.0f);

	*reinterpret_cast<float*>(address + addresses["relativeOffset"] + 4) =
		g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetY", 0.0f);

	*reinterpret_cast<float*>(address + addresses["relativeOffset"] + 8) =
		g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetZ", 0.0f);

	*reinterpret_cast<bool*>(address + addresses["useReticule"]) =
		g_scriptconfig.get<bool>("OnFootCamera", "EnableReticle", false);
}

void patchCinematicMountedCameraMetadata(uintptr_t address)
{
	AddressPool addresses = (*g_addresses.get("cinematicMountedCamera"));

	*reinterpret_cast<float*>(address + addresses["minPitch"]) = 
		g_scriptconfig.get<float>("InVehicleCamera", "MinPitch", -41.70f);

	*reinterpret_cast<float*>(address + addresses["maxPitch"]) =
		g_scriptconfig.get<float>("InVehicleCamera", "MaxPitch", 30.0f);;

	*reinterpret_cast<float*>(address + addresses["minPitchOverride"]) = 
		*reinterpret_cast<float*>(address + addresses["minPitch"]);

	*reinterpret_cast<float*>(address + addresses["maxPitchOverride"]) = 
		*reinterpret_cast<float*>(address + addresses["maxPitch"]);

	*reinterpret_cast<float*>(address + addresses["fov"]) = 
		g_scriptconfig.get<float>("InVehicleCamera", "FOV", 50.0f);

	*reinterpret_cast<float*>(address + addresses["minSpeedForCorrect"]) = 
		g_scriptconfig.get<float>("InVehicleCamera", "MinSpeedForCorrection", 20.0f);
}

void setGlobalsFromConfigEntries()
{
	followVehicleHighSpeedShake = g_scriptconfig.get<bool>("FollowVehicleCamera", "UseHighSpeedShake", false);

	firstPersonUseReticle = g_scriptconfig.get<bool>("OnFootCamera", "EnableReticle", false);

	firstPersonVehicleFovScale = static_cast<int>((10.0f / 50.0f) * -(g_scriptconfig.get<float>("InVehicleCamera", "FOV", 50.0f) + 50.0f));
}

void patchFollowVehicleCameraMetadata(uintptr_t address)
{
	AddressPool addresses = (*g_addresses.get("followVehicleCamera"));

	*reinterpret_cast<float*>(address + addresses["minSpeedForShake"]) =
		g_scriptconfig.get<bool>("FollowVehicleCamera", "UseHighSpeedShake", false) ? 40.0f : FLT_MAX;
}

void patchMetadataGlobal()
{
	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (camMetadataRef * ref = *it; ref; ref = ref->pNext)
		{
			camBaseObjectMetadata * metadata = ref->pData;

			char * psoStruct = metadata->getPsoStruct();

			auto metadataTypeHash = *(DWORD*)(psoStruct + 8);

			uintptr_t address = reinterpret_cast<uintptr_t>(metadata);

			switch (metadataTypeHash)
			{
			case eCamCinematicMountedCameraMetadata:
				patchCinematicMountedCameraMetadata(address);
				continue;
			case eCamFirstPersonShooterCameraMetadata:
				patchFirstPersonShooterCameraMetadata(address);
				continue;	
			case eCamFollowVehicleCameraMetadata:
				patchFollowVehicleCameraMetadata(address);
				continue;
			}
		}
	}
}

void doFirstPersonEnterWaterJmpPatch()
{
	if (g_scriptconfig.get<bool>("General", "DisableWaterEnterCameraSwap", true))
	{
		g_cinematicCameraEnterWaterPatch1.install();
	}

	if (g_scriptconfig.get<bool>("General", "DisableVehicleDestroyedCameraSwap", true))
	{
		g_cinematicCameraEnterWaterPatch2.install();
	}
}

const char * getGxtEntryForHash_Hook(const char * text, unsigned int hashName)
{
	std::unique_lock<std::mutex> lock(g_textMutex);

	auto it = g_textEntries.find(hashName);

	if (it != g_textEntries.end())
	{
		return it->second.c_str();
	}

	return g_getGxtEntryFn->function(text, hashName);
}

unsigned int addGxtEntry(const char * key, const char * value)
{
	auto hashKey = hashString(key);

	std::unique_lock<std::mutex> lock(g_textMutex);

	g_textEntries[hashKey] = value;

	return hashKey;
}

inline PauseMenuInstance * lookupMenuForIndex(int menuIndex)
{
	for (auto it = activeMenuArray.begin(); it != activeMenuArray.end(); it++)
	{
		if (it && it->menuLutIndex == menuIndex)
		{
			return it;
		}
	}

	return NULL;
}

template <typename T>
T patchMetadataValue(eMetadataHash type, std::string category, std::string key, T value)
{
	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (camMetadataRef * ref = *it; ref; ref = ref->pNext)
		{
			camBaseObjectMetadata * metadata = ref->pData;

			auto metadataTypeHash = *(DWORD*)(metadata->getPsoStruct() + 8);

			uintptr_t address = reinterpret_cast<uintptr_t>(metadata);

			if (metadataTypeHash == type)
			{
				*reinterpret_cast<T*>(address + (*g_addresses.get(category))[key]) = value;
			}
		}
	}

	return value;
}

void SetPauseMenuPreference_Hook(long long settingIndex, int value, unsigned int unk)
{
	if (settingIndex >= 200)
	{
		switch (settingIndex)
		{
		case 200:
			if (value)
				g_cinematicCameraEnterWaterPatch1.remove();
			else g_cinematicCameraEnterWaterPatch1.install();
			return;
		case 201:
			if (value)
				g_cinematicCameraEnterWaterPatch2.remove();
			else g_cinematicCameraEnterWaterPatch2.install();
			return;
		case 202:
			firstPersonUseReticle = patchMetadataValue<bool>(eMetadataHash::eCamFirstPersonShooterCameraMetadata,
				"firstPersonCamera", "useReticule", value != 0);
			return;
		case 203:
			patchMetadataValue<float>(eMetadataHash::eCamCinematicMountedCameraMetadata,
				"cinematicMountedCamera", "fov", 35.0f + (value * 4));
			firstPersonVehicleFovScale = value;
			return;
		case 204:
			patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "minSpeedForShake", value ? 40.0f : FLT_MAX );
			followVehicleHighSpeedShake = value != 0;
			return;			
		default:
			return;
		}
	}

	else return g_setPauseMenuPreferenceFn->function(settingIndex, value, unk);
}

bool SetMenuSlot_Hook(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bSlotUpdate)
{
	if (settingIndex >= 200)
	{
		switch (settingIndex)
		{
		case 200:
			value = g_cinematicCameraEnterWaterPatch1.active;
			break;
		case 201:
			value = g_cinematicCameraEnterWaterPatch2.active;
			break;
		case 202:
			value = firstPersonUseReticle;
			break;
		case 203:
			value = firstPersonVehicleFovScale;
			break;
		case 204:
			value = followVehicleHighSpeedShake;
			break;
		default:
			break;
		}
	}

	return g_createToggleItemFn->function(columnId, slotIndex, menuState, settingIndex, unk, value, text, bPopScaleform, bSlotUpdate);
}

void addPauseMenuItems()
{
	int const cameraSettingsLutIdx = 136;

	PauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	auto newCount = pMenu->itemCount + numPauseMenuItems;

	auto newSize = newCount * sizeof(PauseMenuItemInfo);

	PauseMenuItemInfo * newItemArray = (PauseMenuItemInfo*) malloc(newSize);

	memcpy_s(newItemArray, newSize, pMenu->items, pMenu->itemCount * sizeof(PauseMenuItemInfo));

	int itemIdx = pMenu->itemCount - 1;

	int targetSettingIdx = 200; // hijack the last 50 settings indices with the hope that rockstar won't add more than 15 new settings in the future.. (max is 255)

	PauseMenuItemInfo item;

	item.menuLutIndex = 51; // "SETTINGS_LIST"

	item.stateFlags = 0;

	item.itemType = 2;

	item.textHash = addGxtEntry("MO_CUSTOM1", "First Person Camera Switch on Enter Water");
	item.targetSettingIdx = targetSettingIdx;
	item.actionType = ePauseMenuItemAction::Toggle;

	newItemArray[itemIdx] = item;

	itemIdx++;

	targetSettingIdx++;

	item.textHash = addGxtEntry("MO_CUSTOM2", "First Person Camera Switch on Vehicle Destroyed");
	item.targetSettingIdx = targetSettingIdx;
	item.actionType = ePauseMenuItemAction::Toggle;

	newItemArray[itemIdx] = item;

	itemIdx++;

	targetSettingIdx++;

	item.textHash = addGxtEntry("MO_CUSTOM3", "First Person Always Use Reticle");
	item.targetSettingIdx = targetSettingIdx;
	item.actionType = ePauseMenuItemAction::Toggle;

	newItemArray[itemIdx] = item;

	itemIdx++;

	targetSettingIdx++;

	item.textHash = addGxtEntry("MO_CUSTOM4", "First Person In Vehicle Field of View");
	item.targetSettingIdx = targetSettingIdx;
	item.actionType = ePauseMenuItemAction::Slider;

	newItemArray[itemIdx] = item;

	itemIdx++;

	targetSettingIdx++;

	item.textHash = addGxtEntry("MO_CUSTOM5", "Follow Vehicle High Speed Shake");
	item.targetSettingIdx = targetSettingIdx;
	item.actionType = ePauseMenuItemAction::Toggle;

	newItemArray[itemIdx] = item;

	itemIdx++;

	targetSettingIdx++;

	newItemArray[itemIdx] = pMenu->items[pMenu->itemCount - 1]; // Reset defaults button should be at the bottom.

	itemIdx++;

	pMenu->items = newItemArray;

	pMenu->itemCount = itemIdx;

	pMenu->maxItems = itemIdx;
}

void removePauseMenuItems()
{
	int const cameraSettingsLutIdx = 136;

	PauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	PauseMenuItemInfo * pOriginalItems = pMenu->items;

	auto newCount = pMenu->itemCount - numPauseMenuItems;

	auto newSize = newCount * sizeof(PauseMenuItemInfo);

	PauseMenuItemInfo * newItemArray = new PauseMenuItemInfo[newCount];

	memcpy_s(newItemArray, newSize, pMenu->items, newSize);

	newItemArray[newCount - 1] = pOriginalItems[pMenu->itemCount - 1];

	pMenu->items = newItemArray;

	pMenu->itemCount = newCount;

	pMenu->maxItems = newCount;

	delete pOriginalItems;
}

void scriptMain()
{
	GAMEPLAY::SET_THIS_SCRIPT_CAN_BE_PAUSED(FALSE);

	while (true)
	{
		if ((*g_gameState) == eGameState::Playing)
		{
			patchMetadataGlobal();

			doFirstPersonEnterWaterJmpPatch();

			if (g_scriptconfig.get<bool>("Other", "Notification", false))
			{
				notifyAboveMap("~r~POVCameraUnlocker\n~w~Patched Successfully.");
			}

			break;
		}

		WAIT(0);
	}
}

void main()
{
	auto gameVersion = getGameVersion();

	// invalid game version
	if (gameVersion == eGameVersion::VER_UNK) return;

	// get pointer to game state..
	auto result = Pattern((BYTE*)"\x0F\x29\x74\x24\x00\x85\xDB", "xxxx?xx").get();

	g_gameState = reinterpret_cast<eGameState*>(*reinterpret_cast<int *>(result - 4) + result);

	// jmp patch #1 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x31\x81\x00\x00\x00\x00\xF3\x0F\x10\x44\x24\x00", "xx????xxxxx?").get(76);

	g_cinematicCameraEnterWaterPatch1 = bytepatch_t((BYTE*)result, (gameVersion < v1_0_877_1_STEAM && gameVersion > v1_0_505_2_NOSTEAM) ?
		std::vector<BYTE>(6, NOP) : gameVersion < v1_0_944_2_STEAM ? std::vector<BYTE> { JMPREL_8 } : std::vector<BYTE>(6, NOP)); // jz = jmp
																																  // jmp patch #2 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x44\x8A\xC5\x48\x8B\x0C\xC8", "xxxxxxx").get(48);

	g_cinematicCameraEnterWaterPatch2 = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP)); // jz = nop

	result = Pattern((BYTE*)"\x88\x50\x41\x48\x8B\x47\x40", "xxxxxxx").get(-0x28);

	result = *reinterpret_cast<int *>(result - 4) + result + 6;

	g_metadataCollection = (rage::pgCollection<camMetadataRef*>*)((*reinterpret_cast<int *>(result + 3) + result - 1));

	result = Pattern((BYTE*)"\x48\x85\xC0\x75\x34\x8B\x0D", "xxxxxxx").get(-0x5);

	g_getGxtEntryFn = HookManager::SetCall<GetGlobalTextEntry_t>(result, getGxtEntryForHash_Hook);

	result = Pattern((BYTE*)"\x0F\xB7\x54\x51\x00", "xxxx?").get();

	activeMenuArray = *(rage::pgCollection<PauseMenuInstance>*)(*reinterpret_cast<int *>(result - 4) + result);

	auto pattern = Pattern((BYTE*)"\x83\xFF\x05\x74\x15", "xxxxx");

	g_createSliderItemFn = HookManager::SetCall<SetMenuSlot_t>(pattern.get(-0x1A), SetMenuSlot_Hook); //-0x1A

	g_createToggleItemFn = HookManager::SetCall<SetMenuSlot_t>(pattern.get(0xA8), SetMenuSlot_Hook); // +0xA8

	pattern = Pattern((BYTE*)"\xF2\x0F\x2C\x56\x00", "xxxx?");

	g_setPauseMenuPreferenceFn = HookManager::SetCall<SetPauseMenuPreference_t>(pattern.get(0x20), SetPauseMenuPreference_Hook);

	memset((void*)pattern.get(0x12), 0x90, 6); // always toggle preferences..

	//memset((void*)0x1409EAEA1, 0x90, 6); // run scripts even in pause menu...

	addOffsetsForGameVersion(gameVersion);

	setGlobalsFromConfigEntries();

	addPauseMenuItems();

	scriptMain();
}

void unload()
{
	removePauseMenuItems();

	if (g_getGxtEntryFn)
	{
		delete g_getGxtEntryFn;
		g_getGxtEntryFn = NULL;
	}

	if (g_setPauseMenuPreferenceFn)
	{
		delete g_setPauseMenuPreferenceFn;
		g_setPauseMenuPreferenceFn = NULL;
	}

	if (g_createSliderItemFn)
	{
		delete g_createSliderItemFn;
		g_createSliderItemFn = NULL;
	}

	if (g_createToggleItemFn)
	{
		delete g_createToggleItemFn;
		g_createToggleItemFn = NULL;
	}
}