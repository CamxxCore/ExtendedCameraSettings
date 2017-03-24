
#include "stdinc.h"

typedef int offset_t;

typedef const char *(*GetGlobalTextEntry_t)(const char * text, unsigned int hashName);

typedef void(*SetPauseMenuPreference_t)(long long settingIndex, int value, unsigned int unk);

typedef bool(*SetMenuSlot_t)(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bIsSlotUpdate);

HMODULE hCurrentModule;

AddressMgr g_addresses;

eGameState * g_gameState;

bytepatch_t g_cinematicCameraEnterWaterPatch1,
g_cinematicCameraEnterWaterPatch2;

CallHook<GetGlobalTextEntry_t> * g_getGxtEntryFn;

CallHook<SetPauseMenuPreference_t> * g_setPauseMenuPreferenceFn;

CallHook<SetMenuSlot_t> * g_createSliderItemFn;

CallHook<SetMenuSlot_t> * g_createToggleItemFn;

rage::pgCollection<camMetadataRef*> * g_metadataCollection;

rage::pgCollection<PauseMenuInstance> activeMenuArray;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

CConfig g_scriptconfig = CConfig("ExtendedCameraSettings.ini");

int iFirstPersonVehicleFovScale = 5,
	iFollowVehicleFovScale = 5,
	iFollowVehicleHorizontalPivotScale = 5,
	iFollowVehicleCamHeightScale = 5;

bool bFollowVehicleCamAutoCenter = false,
	 bFollowVehicleCamHighSpeedShake = false,
	 bFollowPedCamRunningShake = false,
	 bFirstPersonPedCamUseReticle = false;

const int kMenuItemsCount = 10;

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
	offsets->insert("useReticle", 114);

#pragma endregion

	#pragma region cinematicMountedCamera

	offsets = g_addresses.getOrCreate("inVehicleCamera");

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

	offsets->insert("fov", 48);
	offsets->insert("minSpeedForShake", gameVer < v1_0_505_2_STEAM ? 1176 : 1192);
	offsets->insert("pivotOffset", gameVer < v1_0_791_2_STEAM ? 232 : 240); //48 8B 81 ? ? ? ? F3 0F 10 B0 ? ? ? ? F3 0F 10 B8 ? ? ? ? 48 8B 05 ? ? ? ? 
	offsets->insert("autoCenterEnabled", gameVer < v1_0_505_2_STEAM ? 877 : 893); //F3 0F 10 8B ? ? ? ? F3 0F 11 44 24 ? F3 0F 10 83 ? ? ? ? F3 0F 5C CA
	offsets->insert("autoCenterLerpScale", gameVer < v1_0_505_2_STEAM ? 892 : 908); //F3 0F 10 88 ? ? ? ? 73 06 
	offsets->insert("followHeight", gameVer < v1_0_791_2_STEAM ? 164 : 172);

#pragma endregion

#pragma region followPedCamera

	offsets = g_addresses.getOrCreate("followPedCamera");

	offsets->insert("minSpeedForShake", gameVer < v1_0_505_2_STEAM ? 2068 : 2092);

#pragma endregion
}

void doFirstPersonEnterWaterJmpPatch()
{
	if (!g_scriptconfig.get<bool>("InVehicleCamera", "SwapCameraOnWaterEnter", true))
	{
		g_cinematicCameraEnterWaterPatch1.install();
	}

	if (!g_scriptconfig.get<bool>("InVehicleCamera", "SwapCameraOnVehicleDestroyed", true))
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

unsigned int addGxtEntry(std::string key, std::string text)
{
	auto hashKey = getHashKey(key.c_str());

	std::unique_lock<std::mutex> lock(g_textMutex);

	g_textEntries[hashKey] = text;

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

void patchFirstPersonShooterCameraMetadata(uintptr_t baseAddress)
{
	AddressPool& addresses = (*g_addresses.get("firstPersonCamera"));

	auto address = baseAddress + addresses["fov"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "FOV", 45.0f));

	address = baseAddress + addresses["minPitch"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "MinPitch", -80.0f));

	address = baseAddress + addresses["maxPitch"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "MaxPitch", 80.0f));

	address = baseAddress + addresses["altMinPitch"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "AltMinPitch", -75.0f));

	address = baseAddress + addresses["altMaxPitch"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "AltMaxPitch", -75.0f));

	address = baseAddress + addresses["altMinYaw"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "AltMinYaw", -45.0f));

	address = baseAddress + addresses["altMaxYaw"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "AltMaxYaw", 45.0f));

	address = baseAddress + addresses["useReticle"];

	WriteBoolean(address, g_scriptconfig.get<bool>("OnFootCamera", "AlwaysUseReticle", false));

	address = baseAddress + addresses["relativeOffset"];

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetX", 0.0f));

	address = address + 4;

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetY", 0.0f));

	address = address + 8;

	WriteFloat(address, g_scriptconfig.get<float>("OnFootCamera", "RelativeOffsetZ", 0.0f));
}

void patchCinematicMountedCameraMetadata(uintptr_t baseAddress)
{
	AddressPool& addresses = (*g_addresses.get("inVehicleCamera"));

	float fPitchValue = g_scriptconfig.get<float>("InVehicleCamera", "MinPitch", -41.70f);

	auto address = baseAddress + addresses["minPitch"];

	WriteFloat(address, fPitchValue);

	address = address + addresses["minPitchOverride"];

	WriteFloat(address, fPitchValue);

	fPitchValue = g_scriptconfig.get<float>("InVehicleCamera", "MaxPitch", 30.0f);

	address = address + addresses["maxPitch"];

	WriteFloat(address, fPitchValue);

	address = address + addresses["maxPitchOverride"];

	WriteFloat(address, fPitchValue);

	address = address + addresses["fov"];

	WriteFloat((address + addresses["fov"]), g_scriptconfig.get<float>("InVehicleCamera", "FOV", 50.0f));

	address = address + addresses["minSpeedForCorrect"];

	WriteFloat(address, g_scriptconfig.get<float>("InVehicleCamera", "MinSpeedForCorrection", 20.0f));
}

void patchFollowVehicleCameraMetadata(uintptr_t baseAddress)
{
	AddressPool& addresses = (*g_addresses.get("followVehicleCamera"));

	auto address = baseAddress + addresses["fov"];

	WriteFloat(address, g_scriptconfig.get<float>("FollowVehicleCamera", "FOV", 50.0f));

	address = baseAddress + addresses["pivotOffset"];

	WriteFloat(address, g_scriptconfig.get<float>("FollowVehicleCamera", "PivotOffsetX", 0.0f));

	WriteFloat((address + 4), g_scriptconfig.get<float>("FollowVehicleCamera", "PivotOffsetY", 0.0f));

	WriteFloat((address + 8), g_scriptconfig.get<float>("FollowVehicleCamera", "PivotOffsetZ", 0.0f));

	address = baseAddress + addresses["minSpeedForShake"];

	WriteFloat(address, g_scriptconfig.get<bool>("FollowVehicleCamera", "UseHighSpeedShake", false) ? 40.0f : FLT_MAX);

	address = address + addresses["autoCenterEnabled"];

	WriteBoolean(address, g_scriptconfig.get<bool>("FollowVehicleCamera", "EnableAutoCenter", false));

	address = address + addresses["followHeight"];

	WriteFloat(address, g_scriptconfig.get<float>("FollowVehicleCamera", "FollowHeight", 1.075f));
}

void patchFollowPedCameraMetadata(uintptr_t baseAddress)
{
	AddressPool& addresses = (*g_addresses.get("followPedCamera"));

	auto address = baseAddress + addresses["minSpeedForShake"];

	WriteFloat(address, g_scriptconfig.get<bool>("FollowPedCamera", "UseRunningShake", false) ? 0.5f : FLT_MAX);
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
				break;
			case eCamFirstPersonShooterCameraMetadata:
				patchFirstPersonShooterCameraMetadata(address);
				break;
			case eCamFollowVehicleCameraMetadata:
				patchFollowVehicleCameraMetadata(address);
				break;
			case eCamFollowPedCameraMetadata:
				patchFollowPedCameraMetadata(address);
				break;
			}
		}
	}
}

void setupMenuItemInfo(PauseMenuItemInfo * item, std::string gxtAlias, std::string text, char type, char actionType, char settingIdx, char stateFlags)
{
	item->textHash = addGxtEntry(gxtAlias, text);

	item->type = type;

	item->actionType = actionType;

	item->targetSettingIdx = settingIdx;

	item->stateFlags = stateFlags;
}

void addPauseMenuItems()
{
	int const cameraSettingsLutIdx = 136;

	PauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	auto newCount = pMenu->itemCount + kMenuItemsCount;

	auto newSize = newCount * sizeof(PauseMenuItemInfo);

	PauseMenuItemInfo * newItemArray = new PauseMenuItemInfo[newCount];

	memcpy_s(newItemArray, newSize, pMenu->items, pMenu->itemCount * sizeof(PauseMenuItemInfo));

	// overwrite the reset button since we will move it to the bottom later..
	int itemIdx = pMenu->itemCount - 1;

	// hijack the last 50 settings indices with the hope that 
	// rockstar won't add more than 15 new settings in the future.. (max is 255 or CHAR_MAX)
	int targetSettingIdx = 200;

	PauseMenuItemInfo item;

	memset(&item, 0x00, sizeof(PauseMenuItemInfo));

	item.menuLutIndex = 51; // aka "SETTINGS_LIST"

	item.stateFlags = 0;

	// add menu settings ->

	item.type = 2; // dynamic type

	setupMenuItemInfo(&item, "MO_CUSTOMXYY", "First Person Vehicle Field of View", 2, eDynamicMenuAction::Slider, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMXZY", "First Person Vehicle Camera Switch on Enter Water", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMZXX", "First Person Vehicle Camera Switch on Destroyed", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMXZZ", "Third Person Vehicle Field of View", 2, eDynamicMenuAction::Slider, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMXXY", "Third Person Vehicle Auto Center", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMZXY", "Third Person Vehicle Horizontal Origin", 2, eDynamicMenuAction::Slider, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMZYZ", "Third Person Vehicle High Speed Shake", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMZXZ", "Third Person Vehicle Follow Height", 2, eDynamicMenuAction::Slider, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMZYY", "Third Person Ped Running Shake", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	setupMenuItemInfo(&item, "MO_CUSTOMXYZ", "First Person Always Use Reticle", 2, eDynamicMenuAction::Toggle, targetSettingIdx++, 0);
	newItemArray[itemIdx] = item;

	itemIdx++;

	// move reset button to the bottom
	newItemArray[itemIdx] = pMenu->items[pMenu->itemCount - 1];

	itemIdx++;

	pMenu->items = newItemArray;

	pMenu->itemCount = itemIdx;

	pMenu->maxItems = itemIdx;
}

void SetPauseMenuPreference_Hook(long long settingIndex, int value, unsigned int unk)
{
	if (settingIndex >= 200)
	{
		switch (settingIndex)
		{
		case 200:
		{
			float realValue = 35.0f + (value * 4);

			patchMetadataValue<float>(eCamCinematicMountedCameraMetadata, "inVehicleCamera", "fov", realValue);

			g_scriptconfig.set<float>("InVehicleCamera", "FOV", realValue);

			iFirstPersonVehicleFovScale = value;

			break;
		}

		case 201:
		{
			value ? g_cinematicCameraEnterWaterPatch1.remove() : g_cinematicCameraEnterWaterPatch1.install();

			g_scriptconfig.set<bool>("InVehicleCamera", "SwapCameraOnWaterEnter", value != 0);

			break;
		}

		case 202:
		{
			value ? g_cinematicCameraEnterWaterPatch2.remove() : g_cinematicCameraEnterWaterPatch2.install();

			g_scriptconfig.set<bool>("InVehicleCamera", "SwapCameraOnVehicleDestroyed", value != 0);

			break;
		}
	
		case 203:
		{
			float realValue = 35.0f + (value * 4);

			patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "fov", realValue);

			g_scriptconfig.set<float>("FollowVehicleCamera", "FOV", realValue);

			iFollowVehicleFovScale = value;

			break;
		}

		case 204:
		{
			bFollowVehicleCamAutoCenter = value != 0;

			patchMetadataValue<bool>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "autoCenterEnabled", bFollowVehicleCamAutoCenter);

			g_scriptconfig.set<bool>("FollowVehicleCamera", "EnableAutoCenter", bFollowVehicleCamAutoCenter);

			break;
		}

		case 205:
		{
			float realValue = ((2.0f / 10.0f) * value) - 1.0f;

			patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "pivotOffset", realValue);

			g_scriptconfig.set<float>("FollowVehicleCamera", "PivotOffsetX", realValue);

			iFollowVehicleHorizontalPivotScale = value;

			break;
		}

		case 206:
		{
			bFollowVehicleCamHighSpeedShake = value != 0;

			patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "", bFollowVehicleCamHighSpeedShake ? 40.0f : FLT_MAX);

			g_scriptconfig.set<bool>("FollowVehicleCamera", "UseHighSpeedShake", bFollowVehicleCamHighSpeedShake);

			break;
		}

		case 207:
		{
			float realValue = 0.4f + (((1.1f / 10.0f) * value));

			patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata,
				"followVehicleCamera", "followHeight", realValue);

			g_scriptconfig.set<float>("FollowVehicleCamera", "FollowHeight", realValue);

			iFollowVehicleCamHeightScale = value;

			break;
		}

		case 208:
		{
			bFollowPedCamRunningShake = value != 0;

			patchMetadataValue<float>(eMetadataHash::eCamFollowPedCameraMetadata,
				"followPedCamera", "minSpeedForShake", bFollowPedCamRunningShake ? 0.5f : FLT_MAX);

			g_scriptconfig.set<bool>("FollowPedCamera", "UseRunningShake", bFollowPedCamRunningShake);

			break;
		}

		case 209:
		{
			bFirstPersonPedCamUseReticle = value != 0;

			patchMetadataValue<bool>(eCamFirstPersonShooterCameraMetadata, "firstPersonCamera", "useReticle", bFirstPersonPedCamUseReticle);

			g_scriptconfig.set<bool>("OnFootCamera", "AlwaysUseReticle", bFirstPersonPedCamUseReticle);

			break;
		}

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
			value = iFirstPersonVehicleFovScale;
			break;
		case 201:
			value = !g_cinematicCameraEnterWaterPatch1.active;
			break;
		case 202:
			value = !g_cinematicCameraEnterWaterPatch2.active;
			break;	
		case 203:
			value = iFollowVehicleFovScale;
			break;
		case 204:
			value = bFollowVehicleCamAutoCenter;
			break;
		case 205:
			value = iFollowVehicleHorizontalPivotScale;
			break;
		case 206:
			value = bFollowVehicleCamHighSpeedShake;
			break;
		case 207:
			value = iFollowVehicleCamHeightScale;
			break;
		case 208:
			value = bFollowPedCamRunningShake;
			break;
		case 209:
			value = bFirstPersonPedCamUseReticle;
			break;
		default:
			break;
		}
	}

	return g_createToggleItemFn->function(columnId, slotIndex, menuState, settingIndex, unk, value, text, bPopScaleform, bSlotUpdate);
}

void setGlobalsFromConfigEntries()
{
	iFirstPersonVehicleFovScale = (int) min(10.0f / 40.0f * (g_scriptconfig.get<float>("InVehicleCamera", "FOV", 50.0f) - 35.0f), 10.0f);

	iFollowVehicleFovScale = (int) min(10.0f / 40.0f * (g_scriptconfig.get<float>("FollowVehicleCamera", "FOV", 50.0f) - 35.0f), 10.0f);

	iFollowVehicleHorizontalPivotScale = (int) min((10.0f / 4.0f * (g_scriptconfig.get<float>("FollowVehicleCamera", "PivotOffsetX", 0.0f) + 2.0f)), 10.0f);

	bFollowVehicleCamAutoCenter = g_scriptconfig.get<bool>("FollowVehicleCamera", "EnableAutoCenter", false);

	bFollowVehicleCamHighSpeedShake = g_scriptconfig.get<bool>("FollowVehicleCamera", "UseHighSpeedShake", false);

	bFollowPedCamRunningShake = g_scriptconfig.get<bool>("FollowPedCamera", "UseRunningShake", false);

	bFirstPersonPedCamUseReticle = g_scriptconfig.get<bool>("OnFootCamera", "AlwaysUseReticle", false);
}

void doPatches()
{
	patchMetadataGlobal();

	doFirstPersonEnterWaterJmpPatch();

	if (g_scriptconfig.get<bool>("General", "Notification", false))
	{
		notifyAboveMap("~r~ExtendedCameraSettings\n~w~Loaded");
	}
}

std::string getResourceConfigData(HMODULE hModule)
{
	std::string result;

	HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_TEXT1), RT_RCDATA);

	if (hRes != NULL)
	{
		HGLOBAL hData = LoadResource(hModule, hRes);

		if (hData != NULL)
		{
			DWORD dataSize = SizeofResource(hModule, hRes);
			char* data = (char*)LockResource(hData);
			result.assign(data, dataSize);
		}
	}

	return result;
}

void makeConfigFile()
{
	if (!std::ifstream(g_scriptconfig.filename))
	{
		notifyAboveMap("~r~ExtendedCameraSettings\n~w~Creating config file...");

		std::string resText = getResourceConfigData(hCurrentModule);

		if (!resText.empty())
		{
			std::ofstream ofs(g_scriptconfig.filename);

			ofs << resText;

			ofs.flush();

			ofs.close();
		}
	}
}

void scriptMain()
{
	while (true)
	{
		if ((*g_gameState) == eGameState::Playing)
		{
			doPatches();

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

	makeConfigFile();

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

	Pattern pattern = Pattern((BYTE*)"\x83\xFF\x05\x74\x15", "xxxxx");

	g_createSliderItemFn = HookManager::SetCall<SetMenuSlot_t>(pattern.get(-0x1A), SetMenuSlot_Hook); //-0x1A

	g_createToggleItemFn = HookManager::SetCall<SetMenuSlot_t>(pattern.get(0xA8), SetMenuSlot_Hook); // +0xA8

	pattern = Pattern((BYTE*)"\xF2\x0F\x2C\x56\x00", "xxxx?");

	g_setPauseMenuPreferenceFn = HookManager::SetCall<SetPauseMenuPreference_t>(pattern.get(0x20), SetPauseMenuPreference_Hook);

	// always toggle preferences
	memset((void*)pattern.get(0x12), 0x90, 6); 

	addOffsetsForGameVersion(gameVersion);

	setGlobalsFromConfigEntries();

	addPauseMenuItems();

	if ((*g_gameState) == eGameState::Playing)
	{
		doPatches();
	}

	else
	{
		scriptMain();
	}
}

void removePauseMenuItems()
{
	int const cameraSettingsLutIdx = 136;

	PauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	PauseMenuItemInfo * pOriginalItems = pMenu->items;

	auto newCount = pMenu->itemCount - kMenuItemsCount;

	auto newSize = newCount * sizeof(PauseMenuItemInfo);

	PauseMenuItemInfo * newItemArray = new PauseMenuItemInfo[newCount];

	memcpy_s(newItemArray, newSize, pMenu->items, newSize);

	// restore reset button
	newItemArray[newCount - 1] = pOriginalItems[pMenu->itemCount - 1];

	pMenu->items = newItemArray;

	pMenu->itemCount = newCount;

	pMenu->maxItems = newCount;

	delete[] pOriginalItems;
}

void removePatches()
{
	if (g_cinematicCameraEnterWaterPatch1.active)
	{
		g_cinematicCameraEnterWaterPatch1.remove();
	}

	if (g_cinematicCameraEnterWaterPatch2.active)
	{
		g_cinematicCameraEnterWaterPatch2.remove();
	}
}

void removeHooks()
{
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

void unload()
{
	removePauseMenuItems();

	removePatches();

	removeHooks();
}