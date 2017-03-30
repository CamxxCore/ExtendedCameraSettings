
#include "stdinc.h"

#pragma warning(disable : 4244 4305) 

typedef int offset_t;

typedef const char *(*GetGlobalTextEntry_t)(const char * text, unsigned int hashName);

typedef void(*SetPauseMenuPreference_t)(long long settingIndex, int value, unsigned int unk);

typedef bool(*SetMenuSlot_t)(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bIsSlotUpdate);

typedef void(*ResetCameraProfileSettings_t)();

typedef void(*CMenuItemInvokedCallback)(int settingIndex, int value);

HMODULE hCurrentModule;

AddressMgr g_addresses;

eGameState * g_gameState;

bytepatch_t g_cinematicCameraEnterWaterPatch1,
g_cinematicCameraEnterWaterPatch2;

CallHook<GetGlobalTextEntry_t> * g_getGxtEntryFn;

CallHook<SetPauseMenuPreference_t> * g_setPauseMenuPreferenceFn;

CallHook<SetMenuSlot_t> * g_createSliderItemFn;

CallHook<SetMenuSlot_t> * g_createToggleItemFn;

CallHook<ResetCameraProfileSettings_t> * g_resetCameraSettingsFn;

rage::pgCollection<camMetadataRef*> * g_metadataCollection;

rage::pgCollection<CPauseMenuInstance> g_activeMenuArray;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

Logger g_logfile("ExtendedCameraSettings.log");

CConfig g_scriptconfig = CConfig("ExtendedCameraSettings.ini");

const int kMenuItemsCount = 14;

struct CustomMenuPref
{
	CMenuItemInvokedCallback m_callback;

	int m_value, m_resetvalue;

	CustomMenuPref(CMenuItemInvokedCallback callback, int value, int resetValue) :
		m_callback(callback),
		m_value(value),
		m_resetvalue(resetValue) {}
};

std::map<int, CustomMenuPref> g_customPrefs;

std::map<int, std::string> g_metadataAddrMap = {
	{ eCamFirstPersonShooterCameraMetadata, "firstPersonCamera" },
	{ eCamFollowVehicleCameraMetadata, "followVehicleCamera" },
	{ eCamFollowPedCameraMetadata, "followPedCamera" },
	{ eCamCinematicMountedCameraMetadata, "inVehicleCamera" },
};

void addOffsetsForGameVersion(int gameVer)
{
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
	offsets->insert("relativeOffsetZ", offsets->map["relativeOffset"] + 8);

#pragma endregion

#pragma region followVehicleCamera

	offsets = g_addresses.getOrCreate("followVehicleCamera");

	offsets->insert("fov", 48);
	offsets->insert("minSpeedForShake", gameVer < v1_0_505_2_STEAM ? 1176 : 1192);
	offsets->insert("pivotOffset", gameVer < v1_0_791_2_STEAM ? 232 : 240); //48 8B 81 ? ? ? ? F3 0F 10 B0 ? ? ? ? F3 0F 10 B8 ? ? ? ? 48 8B 05 ? ? ? ? 
	offsets->insert("autoCenterEnabled", gameVer < v1_0_505_2_STEAM ? 877 : 893); //F3 0F 10 8B ? ? ? ? F3 0F 11 44 24 ? F3 0F 10 83 ? ? ? ? F3 0F 5C CA
	offsets->insert("autoCenterLerpScale", gameVer < v1_0_505_2_STEAM ? 892 : 908); //F3 0F 10 88 ? ? ? ? 73 06 
	offsets->insert("followDistance", gameVer < v1_0_791_2_STEAM ? 312 : 320); // 350 312
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

const char * getGxtEntry_Stub(const char * text, unsigned int hashName)
{
	std::unique_lock<std::mutex> lock(g_textMutex);

	auto it = g_textEntries.find(hashName);

	if (it != g_textEntries.end())
	{
		return it->second.c_str();
	}

	return g_getGxtEntryFn->fn(text, hashName);
}

unsigned int addGxtEntry(std::string key, std::string text)
{
	auto hashKey = getHashKey(key.c_str());

	std::unique_lock<std::mutex> lock(g_textMutex);

	g_textEntries[hashKey] = text;

	return hashKey;
}

inline CPauseMenuInstance * lookupMenuForIndex(int menuIndex)
{
	for (auto it = g_activeMenuArray.begin(); it != g_activeMenuArray.end(); it++)
	{
		if (it && it->menuIndex == menuIndex)
		{
			return it;
		}
	}

	return NULL;
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

	address = baseAddress + addresses["relativeOffset"];

	WriteFloat(address, g_scriptconfig.get<float>("InVehicleCamera", "RelativeOffsetX", 0.0f));

	address = address + 4;

	WriteFloat(address, g_scriptconfig.get<float>("InVehicleCamera", "RelativeOffsetY", 0.0f));

	address = address + 8;

	WriteFloat(address, g_scriptconfig.get<float>("InVehicleCamera", "RelativeOffsetZ", 0.0f));

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

	address = address + addresses["followDistance"];

	WriteFloat(address, g_scriptconfig.get<float>("FollowVehicleCamera", "FollowDistance", 1.0f));

	address = address + addresses["followHeight"];

	WriteFloat(address, g_scriptconfig.get<float>("FollowVehicleCamera", "FollowHeight", 1.075f));
}

void patchFollowPedCameraMetadata(uintptr_t baseAddress)
{
	AddressPool& addresses = (*g_addresses.get("followPedCamera"));

	auto address = baseAddress + addresses["minSpeedForShake"];

	WriteFloat(address, g_scriptconfig.get<bool>("FollowPedCamera", "UseRunningShake", false) ? 0.5f : FLT_MAX);
}

template <typename T>
T patchMetadataValue(eMetadataHash type, std::string key, T value)
{
	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (camMetadataRef * ref = *it; ref; ref = ref->pNext)
		{
			camBaseObjectMetadata * metadata = ref->pData;

			if (metadata && *(uintptr_t**)(metadata))
			{
				auto metadataTypeHash = *(Hash*)(metadata->getPsoStruct() + 8);

				uintptr_t address = reinterpret_cast<uintptr_t>(metadata);

				if (metadataTypeHash == type)
				{
					*reinterpret_cast<T*>(address + (*g_addresses.get(g_metadataAddrMap[type]))[key]) = value;
				}
			}
		}
	}

	return value;
}

template <typename T>
bool patchMetadataValues(eMetadataHash type, std::string format, const std::vector<T> & args)
{
	if (format.empty() || args.empty()) return false;

	std::vector<std::string> parameters;

	split(format, ";", parameters);

	if (parameters.size() != args.size()) return false;

	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (camMetadataRef * ref = *it; ref; ref = ref->pNext)
		{
			camBaseObjectMetadata * metadata = ref->pData;

			if (metadata && *(uintptr_t**)(metadata))
			{
				auto metadataTypeHash = *(DWORD*)(metadata->getPsoStruct() + 8);

				uintptr_t address = reinterpret_cast<uintptr_t>(metadata);

				if (metadataTypeHash == type)
				{
					for (size_t i = 0; i < parameters.size(); i++)
					{
						*reinterpret_cast<T*>(address + (*g_addresses.get(g_metadataAddrMap[type]))[parameters[i]]) = args[i];
					}
				}
			}
		}
	}

	return true;
}

void patchMetadataGlobal()
{
	g_logfile.write("patchMetadataGlobal(): Begin patching metadata collection...");

	g_logfile.write("patchMetadataGlobal(): %d metadata entries found.", g_metadataCollection->m_count);

	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (camMetadataRef * ref = *it; ref; ref = ref->pNext)
		{
			camBaseObjectMetadata * metadata = ref->pData;

			if (metadata && *(uintptr_t**)(metadata))
			{
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

	g_logfile.write("patchMetadataGlobal(): Finished patching with no error.");
}

void setupMenuItem(CPauseMenuItem * item, std::string gxtAlias, std::string text, int menuIndex, int type, int actionType, int settingIdx, int stateFlags)
{
	memset(item, 0x00, sizeof(CPauseMenuItem));

	item->textHash = addGxtEntry(gxtAlias, text);
	item->menuIndex = menuIndex;
	item->type = type;
	item->actionType = actionType;
	item->targetSettingIdx = (unsigned char)settingIdx;
	item->stateFlags = stateFlags;

	g_logfile.write("setupMenuItem(): Populating item with type '%d' action '%d' GXT alias \"%s\" display name \"%s\"",
		type,
		actionType,
		gxtAlias.c_str(),
		text.c_str());
}

void addPauseMenuItems()
{
	g_logfile.write("addPauseMenuItems(): Begin add menu items...");

	int const cameraSettingsLutIdx = 136;

	g_logfile.write("addPauseMenuItems(): Performing lookup for menu index (%d)...", cameraSettingsLutIdx);

	CPauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	auto newCount = pMenu->itemCount + kMenuItemsCount;

	auto newSize = newCount * sizeof(CPauseMenuItem);

	CPauseMenuItem * newItemArray = new CPauseMenuItem[newCount];

	memcpy_s(newItemArray, newSize, pMenu->items, pMenu->itemCount * sizeof(CPauseMenuItem));

	// overwrite the reset button since we will move it to the bottom later..
	int itemIdx = pMenu->itemCount - 1;

	// hijack the last 50 settings indices with the hope that 
	// rockstar won't add more than 15 new settings in the future.. (max is 255 or CHAR_MAX)
	int targetSettingIdx = 200;

	// add menu settings (kMenuItemsCount needs to be updated to reflect changes made here)
	//
	#pragma region FP FOV

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYY", "First Person Vehicle Field of View", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		float realValue = ScalarToFloat(value, 30.0f, 70.0f);

		patchMetadataValue<float>(eMetadataHash::eCamCinematicMountedCameraMetadata, "fov", realValue);

		g_scriptconfig.set<float>("InVehicleCamera", "FOV", realValue);

	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("InVehicleCamera", "FOV", 50.0f), 30.0f, 70.0f), 10.0f), 5)));

	#pragma endregion

	#pragma region FP Vertical Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXX", "First Person Vehicle Vertical Origin", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		float realValue = ScalarToFloat(value, -0.8f, 0.8f);

		patchMetadataValue<float>(eMetadataHash::eCamCinematicMountedCameraMetadata, "relativeOffsetZ", realValue);

		g_scriptconfig.set<float>("InVehicleCamera", "RelativeOffsetZ", realValue);

	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("InVehicleCamera", "RelativeOffsetZ", 0.0f), -0.8f, 0.8f), 10.0f), 5)));

	#pragma endregion

	#pragma region FP Switch In Water

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZY", "First Person Vehicle Switch In Water", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		value ? g_cinematicCameraEnterWaterPatch1.remove() : g_cinematicCameraEnterWaterPatch1.install();

		g_scriptconfig.set<bool>("InVehicleCamera", "SwapCameraOnWaterEnter", value != 0);
	}, !g_cinematicCameraEnterWaterPatch1.active, 1)));

	#pragma endregion

	#pragma region FP Switch On Desroyed

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXX", "First Person Vehicle Switch on Destroyed", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		value ? g_cinematicCameraEnterWaterPatch2.remove() : g_cinematicCameraEnterWaterPatch2.install();

		g_scriptconfig.set<bool>("InVehicleCamera", "SwapCameraOnVehicleDestroyed", value != 0);
	}, !g_cinematicCameraEnterWaterPatch2.active, 1)));

	#pragma endregion

	#pragma region FP Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZX", "First Person Vehicle Min Pitch Limit", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		float realValue = -ScalarToFloat(value, -25.0f, 55.0f);

		patchMetadataValues<float>(eCamCinematicMountedCameraMetadata, "minPitch;minPitchOverride", std::vector<float>(2, realValue));

		g_scriptconfig.set<float>("InVehicleCamera", "MinPitch", realValue);

	}, min(FloatToScalar(-g_scriptconfig.get<float>("InVehicleCamera", "MinPitch", -15.0f), -25.0, 55.0), 10.0), 5)));

	#pragma endregion

	#pragma region FP Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYZX", "First Person Vehicle Max Pitch Limit", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		float realValue = ScalarToFloat(value, -25.0f, 55.0f);

		patchMetadataValues<float>(eCamCinematicMountedCameraMetadata, "maxPitch;maxPitchOverride", std::vector<float>(2, realValue));

		g_scriptconfig.set<float>("InVehicleCamera", "MaxPitch", realValue);

	}, min(FloatToScalar(g_scriptconfig.get<float>("InVehicleCamera", "MaxPitch", 15.0f), -25.0, 55.0), 10.0), 5)));

	#pragma endregion

	#pragma region TPV Field Of View

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZZ", "Third Person Vehicle Field of View", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		float realValue = ScalarToFloat(value, 30.0f, 70.0f);// 30.0f + (value * 4);

		patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata, "fov", realValue);

		g_scriptconfig.set<float>("FollowVehicleCamera", "FOV", realValue);

	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("FollowVehicleCamera", "FOV", 50.0f), 30.0f, 70.0f), 10.0f), 5)));

	#pragma endregion

	#pragma region TPV Auto Center

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXXY", "Third Person Vehicle Auto Center", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		patchMetadataValue<bool>(eMetadataHash::eCamFollowVehicleCameraMetadata, "autoCenterEnabled", value != 0);

		g_scriptconfig.set<bool>("FollowVehicleCamera", "EnableAutoCenter", value != 0);

	}, g_scriptconfig.get<bool>("FollowVehicleCamera", "EnableAutoCenter", false), 1)));

	#pragma endregion

	#pragma region TPV Follow Distance

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZX", "Third Person Vehicle Follow Distance", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		float realValue = ScalarToFloat(value, -0.925f, 3.075f);// ((2.15f / 10.0f) * value);

		patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata, "followDistance", realValue);

		g_scriptconfig.set<float>("FollowVehicleCamera", "FollowDistance", realValue);
	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("FollowVehicleCamera", "FollowDistance", 1.075f), -0.925f, 3.075f), 10.0f), 5)));

	#pragma endregion

	#pragma region TPV Follow Height

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXZ", "Third Person Vehicle Pivot Scale", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		float realValue = ScalarToFloat(value, 0, 2.0f);

		patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata, "followHeight", realValue);

		g_scriptconfig.set<float>("FollowVehicleCamera", "FollowHeight", realValue);

	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("FollowVehicleCamera", "FollowHeight", 1.0f), 0.0f, 2.0f), 10.0f), 5)));

	#pragma endregion

	#pragma region TPV Horizontal Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXY", "Third Person Vehicle Horizontal Origin", 51, 2, eDynamicMenuAction::Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		float realValue = ScalarToFloat(value, -2.0f, 2.0f);

		patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata, "pivotOffset", realValue);

		g_scriptconfig.set<float>("FollowVehicleCamera", "PivotOffsetX", realValue);

	}, (int)min(FloatToScalar(g_scriptconfig.get<float>("FollowVehicleCamera", "PivotOffsetX", 0.0f) , -2.0f, 2.0f), 10.0f), 5)));

	#pragma endregion

	#pragma region TPV High Speed Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYZ", "Third Person Vehicle High Speed Shake", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		patchMetadataValue<float>(eMetadataHash::eCamFollowVehicleCameraMetadata, "minSpeedForShake", value != 0 ? 40.0f : FLT_MAX);

		g_scriptconfig.set<bool>("FollowVehicleCamera", "UseHighSpeedShake", value != 0);

	}, g_scriptconfig.get<bool>("FollowVehicleCamera", "UseHighSpeedShake", false), 1)));

	#pragma endregion

	#pragma region TPP Running Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYY", "Third Person Ped Running Shake", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		patchMetadataValue<float>(eMetadataHash::eCamFollowPedCameraMetadata, "minSpeedForShake", value != 0 ? 0.5f : FLT_MAX);

		g_scriptconfig.set<bool>("FollowPedCamera", "UseRunningShake", value != 0);
	}, g_scriptconfig.get<bool>("FollowPedCamera", "UseRunningShake", false), 1)));

	#pragma endregion

	#pragma region FP Use Reticle

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYZ", "First Person Always Use Reticle", 51, 2, eDynamicMenuAction::Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		patchMetadataValue<bool>(eMetadataHash::eCamFirstPersonShooterCameraMetadata, "useReticle", value != 0);

		g_scriptconfig.set<bool>("OnFootCamera", "AlwaysUseReticle", value != 0);

	}, g_scriptconfig.get<bool>("OnFootCamera", "AlwaysUseReticle", false), 0)));

	#pragma endregion

	// move reset button to the bottom
	newItemArray[itemIdx] = pMenu->items[pMenu->itemCount - 1];

	itemIdx++;

	g_logfile.write("addPauseMenuItems(): Expanding pause menu array...");

	pMenu->items = newItemArray;

	pMenu->itemCount = itemIdx;

	pMenu->maxItems = itemIdx;

	g_logfile.write("addPauseMenuItems(): Finished adding menu items!");
}

bool SetMenuSlot_Stub(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bSlotUpdate)
{
	if (settingIndex >= 200)
	{
		auto it = g_customPrefs.find(settingIndex);

		if (it != g_customPrefs.end())
		{
			value = it->second.m_value;
		}
	}

	return g_createToggleItemFn->fn(columnId, slotIndex, menuState, settingIndex, unk, value, text, bPopScaleform, bSlotUpdate);
}

void SetPauseMenuPreference_Stub(long long settingIndex, int value, unsigned int unk)
{
	if (settingIndex >= 200)
	{
		int prefId = (int)settingIndex;

		auto it = g_customPrefs.find(prefId);

		if (it != g_customPrefs.end())
		{
			it->second.m_value = value;

			if (it->second.m_callback)
			{
				it->second.m_callback(prefId, value);
			}
		}
	}

	else g_setPauseMenuPreferenceFn->fn(settingIndex, value, unk);
}

void ResetCameraProfileSettings_Stub()
{
	g_resetCameraSettingsFn->fn();

	for (auto it = g_customPrefs.begin(); it != g_customPrefs.end(); it++)
	{
		SetPauseMenuPreference_Stub(it->first, it->second.m_resetvalue, 3);
	}
}

void doPatches()
{
	g_logfile.write("doPatches(): Begin patches...");

	patchMetadataGlobal();

	doFirstPersonEnterWaterJmpPatch();

	if (g_scriptconfig.get<bool>("General", "Notification", false))
	{
		printToScreen("Loaded");
	}

	g_logfile.write("doPatches(): Patches completed!");
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
	if (!std::ifstream(g_scriptconfig.filename, std::ifstream::in))
	{
		g_logfile.write("makeConfigFile(): Creating new config...");

		printToScreen("Creating config file...");

		std::string resText = getResourceConfigData(hCurrentModule);

		if (!resText.empty())
		{
			std::ofstream ofs(g_scriptconfig.filename);

			if (ofs.good())
			{
				ofs << resText;
				ofs.flush();
				ofs.close();
			}
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

	addOffsetsForGameVersion(gameVersion);

	// get pointer to game state..
	auto result = Pattern((BYTE*)"\x0F\x29\x74\x24\x00\x85\xDB", "xxxx?xx").get();

	if (result)
	{
		g_logfile.write("main(): g_gameState found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find g_gameState");
		return;
	}

	g_gameState = reinterpret_cast<eGameState*>(*reinterpret_cast<int *>(result - 4) + result);

	// jmp patch #1 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x31\x81\x00\x00\x00\x00\xF3\x0F\x10\x44\x24\x00", "xx????xxxxx?").get(76);

	if (result)
	{
		g_logfile.write("main(): g_enterWaterPatch1 found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find g_enterWaterPatch1");
		return;
	}

	g_cinematicCameraEnterWaterPatch1 = bytepatch_t((BYTE*)result, (gameVersion < v1_0_877_1_STEAM && gameVersion > v1_0_505_2_NOSTEAM) ?
		std::vector<BYTE>(6, NOP) : gameVersion < v1_0_944_2_STEAM ? std::vector<BYTE> { JMPREL_8 } : std::vector<BYTE>(6, NOP)); // jz = jmp		
																																  // jmp patch #2 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x44\x8A\xC5\x48\x8B\x0C\xC8", "xxxxxxx").get(48);

	if (result)
	{
		g_logfile.write("main(): g_enterWaterPatch2 found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find g_enterWaterPatch2");
		return;
	}

	g_cinematicCameraEnterWaterPatch2 = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP)); // jz = nop

	result = Pattern((BYTE*)"\x88\x50\x41\x48\x8B\x47\x40", "xxxxxxx").get(-0x28);

	if (result)
	{
		g_logfile.write("main(): g_metadataCollection found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find g_metadataCollection");
		return;
	}

	result = *reinterpret_cast<int *>(result - 4) + result + 6;

	g_metadataCollection = (rage::pgCollection<camMetadataRef*>*)((*reinterpret_cast<int *>(result + 3) + result - 1));

	result = Pattern((BYTE*)"\x0F\xB7\x54\x51\x00", "xxxx?").get();

	if (result)
	{
		g_logfile.write("main(): g_activeMenuArray found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find g_activeMenuArray");
		return;
	}

	g_activeMenuArray = *(rage::pgCollection<CPauseMenuInstance>*)(*reinterpret_cast<int *>(result - 4) + result);

	result = Pattern((BYTE*)"\x48\x85\xC0\x75\x34\x8B\x0D", "xxxxxxx").get(-0x5);

	if (result)
	{
		g_logfile.write("main(): getGxtEntry() found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find getGxtEntry()");
		return;
	}

	g_getGxtEntryFn = HookManager::SetCall<GetGlobalTextEntry_t>(result, getGxtEntry_Stub);

	Pattern pattern = Pattern((BYTE*)"\x83\xFF\x05\x74\x15", "xxxxx");

	result = pattern.get(-0x1A);

	if (result)
	{
		g_logfile.write("main(): SetMenuSlot() #1 found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find setMenuSlot() #1");
		return;
	}

	g_createSliderItemFn = HookManager::SetCall<SetMenuSlot_t>(result, SetMenuSlot_Stub); //-0x1A

	result = pattern.get(0xA8);

	if (result)
	{
		g_logfile.write("main(): SetMenuSlot() #2 found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find SetMenuSlot() #2");
		return;
	}

	g_createToggleItemFn = HookManager::SetCall<SetMenuSlot_t>(result, SetMenuSlot_Stub); // +0xA8

	pattern = Pattern((BYTE*)"\x81\xE9\x00\x00\x00\x00\x74\x25\xFF\xC9", "xx????xxxx");

	result = pattern.get(-0x2A);

	if (result)
	{
		g_logfile.write("main(): ResetCameraProfileSettings() found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find ResetCameraProfileSettings()");
		return;
	}

	g_resetCameraSettingsFn = HookManager::SetCall<ResetCameraProfileSettings_t>(result, ResetCameraProfileSettings_Stub);

	pattern = Pattern((BYTE*)"\xF2\x0F\x2C\x56\x00", "xxxx?");

	result = pattern.get(0x20);

	if (result)
	{
		g_logfile.write("main(): setPauseMenuPreference() found at 0x%llX", result);
	}

	else
	{
		g_logfile.write("[ERROR] main(): Failed to find SetPauseMenuPreference()");
		return;
	}

	g_setPauseMenuPreferenceFn = HookManager::SetCall<SetPauseMenuPreference_t>(result, SetPauseMenuPreference_Stub);

	result = pattern.get(0x12);

	// always toggle preferences
	memset((void*)pattern.get(0x12), 0x90, 6);

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

	CPauseMenuInstance * pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	CPauseMenuItem * pOriginalItems = pMenu->items;

	auto newCount = pMenu->itemCount - kMenuItemsCount;

	auto newSize = newCount * sizeof(CPauseMenuItem);

	CPauseMenuItem * newItemArray = new CPauseMenuItem[newCount];

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

	if (g_resetCameraSettingsFn)
	{
		delete g_resetCameraSettingsFn;
		g_resetCameraSettingsFn = NULL;
	}
}

void unload()
{
	removePauseMenuItems();

	removePatches();

	removeHooks();
}