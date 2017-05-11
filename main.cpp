
#include "stdafx.h"

#pragma warning(disable : 4244 4305) 

typedef const char *(*GetGlobalTextEntry_t)(const char * text, unsigned int hashName);

typedef void(*SetPauseMenuPreference_t)(long long settingIndex, int value, unsigned int unk);

typedef bool(*SetMenuSlot_t)(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bIsSlotUpdate);

typedef void(*ResetCameraProfileSettings_t)();

typedef camBaseDirector* (*GetCamDirectorFromPool_t)(unsigned int * hashName);

HMODULE g_currentModule;

AddressMgr g_addresses;

eGameState * g_gameState;

bytepatch_t g_cinematicCameraEnterWaterPatch1,
g_cinematicCameraEnterWaterPatch2,
g_autoRotatePatch;

CallHook<GetGlobalTextEntry_t> * g_getGxtEntry;

CallHook<SetPauseMenuPreference_t> * g_setPauseMenuPreference;

CallHook<SetMenuSlot_t> * g_createSliderItem;

CallHook<SetMenuSlot_t> * g_createToggleItem;

CallHook<ResetCameraProfileSettings_t> * g_resetCameraSettings;

rage::pgCollection<camBaseDirector*> * g_camDirectorPool;

rage::pgCollection<camMetadataRef*> * g_metadataCollection;

rage::pgCollection<CPauseMenuInstance> g_activeMenuArray;

GetCamDirectorFromPool_t getCamDirectorFromPool;

camBaseCamera * g_activeCamera;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

CConfig g_scriptconfig = CConfig("ExtendedCameraSettings.ini");

const int kMenuItemsCount = 24;

const int kSettingsStartIndex = 230;

bool bDidLoad = false;
bool bInitialized = false;

bool bUseGlobalPresets = false,
	 bAutoSaveLayouts = false,
	 bShouldUpdatePresets = false,
	 bVehicleCamActive = false,
	 bShouldPlaySound = false;

int lastSoundPlayedTime = 0;

std::map<unsigned int, std::vector<CamMetadataPreset>> g_camPresets;

std::map<int, CustomMenuPref> g_customPrefs;

unsigned int g_modelId;

Entity targetEntity;

camBaseDirector * g_camGameplayDirector = nullptr;

DWORD vkReloadPresets = VK_B, vkSaveLayout = VK_F11;

std::map<int, std::string> g_dataFileTypeMap = {
	{ CPT_BOOLEAN, "bool" },
	{ CPT_INTEGER, "int" },
	{ CPT_UINTEGER, "uint" },
	{ CPT_FLOAT, "float" },
	{ CPT_DOUBLE, "double" }
};

std::map<eMetadataHash, std::string> g_metadataHashMap = {
	{ eCamFirstPersonShooterCameraMetadata, "camFirstPersonShooterCameraMetadata" },
	{ eCamFollowVehicleCameraMetadata, "camFollowVehicleCameraMetadata" },
	{ eCamFollowPedCameraMetadata, "camFollowPedCameraMetadata" },
	{ eCamCinematicMountedCameraMetadata, "camCinematicMountedCameraMetadata" },
	{ eCamThirdPersonPedAimCameraMetadata, "camThirdPersonPedAimCameraMetadata" },
};

void addOffsetsForGameVersion(int gameVer)
{
	#pragma region firstPersonCamera

	auto offsets = g_addresses.getOrCreate("camFirstPersonShooterCameraMetadata");

	offsets->insert("fov", 36); //F3 44 0F 5C 48 ? F3 41 0F 59 F8 
	offsets->insert("minPitch", 84); //F3 0F 10 40 ? F3 0F 59 05 ? ? ? ? F3 0F 11 87 ? ? ? ? 
	offsets->insert("maxPitch", 88); //+4
	offsets->insert("alwaysUseReticle", 114); //44 38 79 72
	offsets->insert("viewOffsetX", 64); //F3 0F 10 4A ? F3 0F 10 42 ? 44 0F 29 ? ? ? ? ?
	offsets->insert("viewOffsetY", offsets->map["viewOffsetX"].add(4));
	offsets->insert("viewOffsetZ", offsets->map["viewOffsetX"].add(8));
	offsets->insert("altMinYaw", gameVer < v1_0_505_2_STEAM ? 696 : gameVer < v1_0_877_1_STEAM ? 712 : 760); //48 8B 81 ? ? ? ? EB E6
	offsets->insert("altMaxYaw", offsets->map["altMinYaw"].add(4)); //+4
	offsets->insert("altMinPitch", offsets->map["altMinYaw"].add(8)); //+8
	offsets->insert("altMaxPitch", offsets->map["altMinYaw"].add(12)); //+C

	#pragma endregion

	#pragma region cinematicMountedCamera

	offsets = g_addresses.getOrCreate("camCinematicMountedCameraMetadata");

	offsets->insert("fov", gameVer < v1_0_877_1_STEAM ? 80 : 84); //F3 0F 10 48 ? F3 0F 11 89 ? ? ? ?
	offsets->insert("minPitch", gameVer < v1_0_505_2_STEAM ? 808 : gameVer < v1_0_877_1_STEAM ? 824 : gameVer < v1_0_944_2_STEAM ? 872 : 888); //F3 0F 10 ? ? ? ? ? 0F 2F D0 72 10 F3 0F 10 ? ? 03 00 00
	offsets->insert("maxPitch", offsets->map["minPitch"].add(4));
	offsets->insert("minPitchExt", gameVer < v1_0_505_2_STEAM ? 776 : gameVer < v1_0_877_1_STEAM ? 792 : gameVer < v1_0_944_2_STEAM ? 840 : 856); //F3 0F 59 87 ? ? ? ? F3 41 0F 59 C4 
	offsets->insert("maxPitchExt", gameVer < v1_0_505_2_STEAM ? 780 : gameVer < v1_0_877_1_STEAM ? 796 : gameVer < v1_0_944_2_STEAM ? 844 : 860);
	offsets->insert("minSpeedForAutoCorrect", gameVer < v1_0_505_2_STEAM ? 680 : gameVer < v1_0_877_1_STEAM ? 696 : gameVer < v1_0_944_2_STEAM ? 744 : 760); //0F 2F B0 ? ? ? ? 0F 82 ? 02 00 00
	offsets->insert("viewOffsetX", gameVer < v1_0_877_1_STEAM ? 80 : 96);  //F3 44 0F 10 ? ? F3 44 0F 10 ? ? F3 44 0F 10 ? ? F3 0F 11 45 ? ? 84 ?
	offsets->insert("viewOffsetY", offsets->map["viewOffsetX"].add(4));
	offsets->insert("viewOffsetZ", offsets->map["viewOffsetX"].add(8));

	#pragma endregion

	#pragma region followVehicleCamera

	offsets = g_addresses.getOrCreate("camFollowVehicleCameraMetadata");

	offsets->insert("fov", 48); //F3 0F 59 48 ? 0F 2F C8 72 2C
	offsets->insert("highSpeedShakeSpeed", gameVer < v1_0_505_2_STEAM ? 1176 : gameVer < v1_0_944_2_STEAM ? 1192 : 1208); //48 81 C7 ? ? ? ? 8B 6F 08 shakesettings+0x10
	offsets->insert("enableAutoCenter", gameVer < v1_0_505_2_STEAM ? 877 : gameVer < v1_0_944_2_STEAM ? 893 : 909); //80 ? ? ? ? ? ? 75 14 48 8B 83 ? ? ? ? 
	offsets->insert("autoCenterLerpScale", gameVer < v1_0_505_2_STEAM ? 892 : gameVer < v1_0_944_2_STEAM ? 908 : 924); //F3 0F 10 88 ? ? ? ? 73 06 
	offsets->insert("followDistance", gameVer < v1_0_791_2_STEAM ? 312 : gameVer < v1_0_944_2_STEAM ? 320 : 328); //F3 0F 10 80 ? ? ? ? C3 4C 8B 81 ? ? ? ? 49 81 C0 ? ? ? ? 
	offsets->insert("pivotScale", gameVer < v1_0_791_2_STEAM ? 164 : gameVer < v1_0_944_2_STEAM ? 172 : 180); //F3 0F 10 88 ? ? ? ? 0F 28 C1 C3 
	offsets->insert("pivotOffsetX", gameVer < v1_0_791_2_STEAM ? 232 : gameVer < v1_0_944_2_STEAM ? 240 : gameVer < v1_0_1011_1_STEAM ? 256 : 248); //F3 0F 10 B0 ? ? ? ? F3 0F 10 B8 ? ? ? ? 48 8B 05 ? ? ? ? 

	#pragma endregion

	#pragma region followPedCamera

	offsets = g_addresses.getOrCreate("camFollowPedCameraMetadata");

	offsets->insert("fov", 48);
	offsets->insert("sprintShakeSpeed", gameVer < v1_0_505_2_STEAM ? 2068 : 2092);
	offsets->insert("minPitch", gameVer < v1_0_505_2_STEAM ? 580 : gameVer < v1_0_944_2_STEAM ? 596 : 612);
	offsets->insert("maxPitch", gameVer < v1_0_505_2_STEAM ? 584 : gameVer < v1_0_944_2_STEAM ? 600 : 616);
	offsets->insert("pivotOffsetX", gameVer < v1_0_791_2_STEAM ? 232 : gameVer < v1_0_944_2_STEAM ? 240 : gameVer < v1_0_1011_1_STEAM ? 256 : 248);
	offsets->insert("followDistance", gameVer < v1_0_791_2_STEAM ? 312 : gameVer < v1_0_944_2_STEAM ? 320 : 328);

	#pragma endregion


	#pragma region pedAimCamera

	offsets = g_addresses.getOrCreate("camThirdPersonPedAimCameraMetadata");

	offsets->insert("fov", 48);
	offsets->insert("pivotOffsetX", gameVer < v1_0_791_2_STEAM ? 232 : gameVer < v1_0_944_2_STEAM ? 240 : gameVer < v1_0_1011_1_STEAM ? 256 : 248);
	offsets->insert("followDistance", gameVer < v1_0_791_2_STEAM ? 312 : gameVer < v1_0_944_2_STEAM ? 320 : 328);

	#pragma endregion

	#pragma region CVehicleModelInfo

	offsets = g_addresses.getOrCreate("CVehicleModelInfo");

	offsets->insert("thirdPersonCameraHash", gameVer < v1_0_505_2_STEAM ? 1120 : gameVer <  v1_0_791_2_STEAM ? 1168 : 1184);
	offsets->insert("firstPersonCameraHash", offsets->map["thirdPersonCameraHash"].add(0xC));
	#pragma endregion
}

void readPresetsFromFile(std::string filename)
{
	g_camPresets.clear();

	tinyxml2::XMLDocument doc;

	auto filePath = Utility::GetWorkingDirectory() + "\\" + filename;

	auto result = doc.LoadFile(filePath.c_str());

	if (result != XML_SUCCESS)
	{
		switch (result)
		{
		case XML_ERROR_FILE_NOT_FOUND:
		{
			LOG("Creating new document %s", filePath.c_str());
			auto pRoot = doc.NewElement("cameraPresets");
			doc.InsertFirstChild(pRoot);
			doc.SaveFile(filePath.c_str());
			break;
		}

		case XML_ERROR_FILE_READ_ERROR:
			LOG("readPresetsFromFile(): Encountered a read error loading the file %s (%d)", filePath.c_str(), result);
			return;

		case XML_ERROR_EMPTY_DOCUMENT:
			LOG("readPresetsFromFile(): Found an xml file but it contains no text!");
			return;

		case XML_ERROR_FILE_COULD_NOT_BE_OPENED:
		default:
			LOG("readPresetsFromFile(): Encountered an error loading the file %s (%d)", filePath.c_str(), result);
			return;
		}
	}

	auto rootElement = doc.FirstChildElement("cameraPresets");

	if (!rootElement)
	{
		LOG("readPresetsFromFile(): No root element.");
		return;
	}

	XMLHelper::ForEach(rootElement, "camPreset", [](XMLElement* e) {

		auto modelName = e->FirstChildElement("modelName")->GetText();

		std::vector<CamMetadataPreset> presets;

		for (auto it = g_metadataHashMap.begin(); it != g_metadataHashMap.end(); ++it)
		{
			auto element = XMLHelper::FirstElement(e, it->second);

			if (!element) continue;

			LOG("Adding %s presets for model %s", it->second.c_str(), modelName);

			for (auto p = element->FirstChildElement("Preset"); p != nullptr; p = p->NextSiblingElement("Preset"))
			{
				CamMetadataPreset preset;

				preset.name = p->Attribute("name");

				preset.metadataHash = static_cast<eMetadataHash>(it->first);

				auto type = p->Attribute("type");

				auto value = p->GetText();

				std::stringstream sstream;

				if (!strcmp(type, "bool"))
				{
					preset.type = CPT_BOOLEAN;

					sstream << std::boolalpha << value;
					sstream >> preset.value.enabled;
				}

				else if (!strcmp(type, "int"))
				{
					preset.type = CPT_INTEGER;

					sstream << value;
					sstream >> preset.value.integer;
				}

				else if (!strcmp(type, "uint"))
				{
					preset.type = CPT_UINTEGER;

					sstream << value;
					sstream >> preset.value.unsignedInt;
				}

				else if (!strcmp(type, "float"))
				{
					preset.type = CPT_FLOAT;

					sstream << value;
					sstream >> preset.value.fvalue;
				}

				else if (!strcmp(type, "double"))
				{
					preset.type = CPT_DOUBLE;

					sstream << value;
					sstream >> preset.value.dvalue;
				}

				presets.push_back(preset);

				LOG("<Preset name=\"%s\" type=\"%s\">%s</Preset>", preset.name.c_str(), type, value);
			}
		}

		g_camPresets.insert(make_pair(parseHashString(modelName), presets));
	});
}

void writePresetsToFile(std::string filename, unsigned int modelHash, std::vector<CamMetadataPreset> presets)
{
	if (!Utility::FileExists(filename))
	{
		LOG("writePresetsToFile(): File not found (%s)", filename.c_str());
		return;
	}

	tinyxml2::XMLDocument doc;

	auto filePath = Utility::GetWorkingDirectory() + "\\" + filename;

	auto result = doc.LoadFile(filePath.c_str());

	if (result != XML_SUCCESS)
	{
		LOG("writePresetsToFile(): Encountered an error loading the file %s (%d)", filePath.c_str(), result);
		return;
	}

	auto rootElement = doc.FirstChildElement("cameraPresets");

	if (!rootElement)
	{
		LOG("writePresetsToFile(): No root element.");
		return;
	}

	auto elem = XMLHelper::FindIf(rootElement, "camPreset", [modelHash](XMLElement* e) -> bool {
		auto str = e->FirstChildElement("modelName")->GetText();
		return modelHash == parseHashString(str);
	});

	if (!elem)
	{
		LOG("writePresetsToFile(): No entry found for hash. Create a new one...");

		elem = doc.NewElement("camPreset");

		auto subNode = doc.NewElement("modelName");

		std::string modelName;

		getVehicleModelName(modelHash, modelName);

		subNode->SetText(modelName.c_str());

		elem->LinkEndChild(subNode);

		rootElement->LinkEndChild(elem);
	}

	LOG("writePresetsToFile(): Setting presets...");

	for (auto it = presets.begin(); it != presets.end(); ++it)
	{
		auto nameStr = g_metadataHashMap[it->metadataHash];

		auto childElem = XMLHelper::FirstElement(elem, nameStr);

		if (!childElem)
		{
			LOG("writePresetsToFile(): Element contains no %s node. Creating a new child node...", nameStr.c_str());

			childElem = doc.NewElement(nameStr.c_str());

			elem->LinkEndChild(childElem);
		}

		auto node = XMLHelper::FindByAttribute(childElem, "Preset", "name", it->name);

		if (node)
		{
			LOG("writePresetsToFile(): Preset node exists. setting value...");

			node->SetText(it->toString().c_str());

			continue;
		}

		LOG("writePresetsToFile(): No preset node found. Creating a new one...");

		node = doc.NewElement("Preset");

		node->SetAttribute("name", it->name.c_str());

		node->SetAttribute("type", g_dataFileTypeMap[it->type].c_str());

		node->SetText(it->toString().c_str());

		childElem->LinkEndChild(node);
	}

	doc.SaveFile(filename.c_str());
}

void writePresetToFile(std::string filename, unsigned int modelHash, CamMetadataPreset preset)
{
	writePresetsToFile(filename, modelHash, std::vector<CamMetadataPreset> { preset });
}

const char * getGxtEntry_Stub(const char * text, unsigned int hashName)
{
	std::unique_lock<std::mutex> lock(g_textMutex);

	auto it = g_textEntries.find(hashName);

	if (it != g_textEntries.end())
	{
		return it->second.c_str();
	}

	return g_getGxtEntry->fn(text, hashName);
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
		if (!it || it->menuId != menuIndex)
			continue;
		return it;
	}

	return nullptr;
}

camBaseObjectMetadata * getCamMetadataForHash(unsigned int hashName)
{
	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (auto ref = *it; ref; ref = ref->pNext)
		{
			if (!ref->pData) continue;

			if (ref->pData->hashKey == hashName)
				return ref->pData;
		}
	}

	return nullptr;
}

void setupMenuItem(CPauseMenuItem * item, std::string gxtAlias, std::string text, int menuIndex, int type, int actionType, int settingIdx, int stateFlags)
{
	item->textHash = addGxtEntry(gxtAlias, text);
	item->menuIndex = menuIndex;
	item->type = type;
	item->actionType = actionType;
	item->targetSettingIdx = static_cast<unsigned char>(settingIdx);
	item->stateFlags = stateFlags;
}

void saveCamPresets()
{
	auto keyValue = bUseGlobalPresets ? 0 : g_modelId;

	auto presets = g_camPresets.find(keyValue);

	if (presets == g_camPresets.end()) return;

	writePresetsToFile("CameraPresets.xml", keyValue, presets->second);

	if (bVehicleCamActive && !bUseGlobalPresets)
	{
		auto szDisplayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(keyValue);

		printToScreen("Settings saved for %s", UI::_GET_LABEL_TEXT(szDisplayName));
	}

	else printToScreen("Camera settings saved");
}

bool checkCamPresetKey(eMetadataHash type, unsigned int * newKeyValue)
{
	if (bUseGlobalPresets)
	{
		*newKeyValue = 0;
	}

	else
	{
		if (bVehicleCamActive && (type == eCamFollowPedCameraMetadata ||
			type == eCamFirstPersonShooterCameraMetadata ||
			type == eCamThirdPersonPedAimCameraMetadata))
		{
			*newKeyValue = ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID());
			return false;
		}

		if (!bVehicleCamActive && (type == eCamFollowVehicleCameraMetadata ||
			type == eCamCinematicMountedCameraMetadata))
		{
			// set global vehicle presets if we aren't sitting in a vehicle.
			*newKeyValue = 0;
			return false;
		}

		*newKeyValue = g_modelId;
	}

	return true;
}

bool getCamPreset(eMetadataHash eType, std::string& name, CamMetadataPreset * preset)
{
	unsigned int keyValue;

	checkCamPresetKey(eType, &keyValue);

	auto items = &g_camPresets[keyValue];

	std::vector<CamMetadataPreset>::iterator it = std::find_if(items->begin(), items->end(),
		[&eType, &name](const CamMetadataPreset&p) {
		return eType == p.metadataHash && name == p.name; });

	if (it == items->end())
		return false;

	LOG("getCamPreset(): Found existing preset %s", it->name.c_str());

	*preset = *it;

	return true;
}

void setCamPresetForKey(unsigned int hashKey, CamMetadataPreset& preset, bool writeToFile)
{
	auto items = &g_camPresets[hashKey];

	auto it = std::find_if(items->begin(), items->end(),
		[&preset](const CamMetadataPreset&p) {
		return preset.metadataHash == p.metadataHash && preset.name == p.name; });

	if (it != items->end())
	{
		it->value = preset.value;
	}

	else
		items->push_back(preset);

	if (!writeToFile) return;

	writePresetToFile("CameraPresets.xml", hashKey, preset);
}

void setCamPreset(CamMetadataPreset& preset)
{
	unsigned int keyValue;

	if (checkCamPresetKey(preset.metadataHash, &keyValue))
		bShouldUpdatePresets = true;

	setCamPresetForKey(keyValue, preset, bAutoSaveLayouts);
}

float getPresetValueFloat(eMetadataHash type, std::string name, float defaultValue)
{
	CamMetadataPreset p;
	if (!getCamPreset(type, name, &p))
		return defaultValue;
	return p.value.fvalue;
}

bool getPresetValueBool(eMetadataHash type, std::string name, bool defaultValue)
{
	CamMetadataPreset p;
	if (!getCamPreset(type, name, &p))
		return defaultValue;
	return p.value.enabled;
}

int getLanguageTextId()
{
	auto uiLanguage = UNK::_GET_UI_LANGUAGE_ID();

	switch (uiLanguage)
	{
	case 0:
		return 0;
	case 4:
		return 1;
	default:
		return 0;
	}
}

void addPauseMenuItems()
{
	LOG("addPauseMenuItems(): Begin add menu items...");

	auto const cameraSettingsLutIdx = 136;

	LOG("addPauseMenuItems(): Performing lookup for menu index (%d)...", cameraSettingsLutIdx);

	auto pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

	auto newCount = pMenu->itemCount + kMenuItemsCount;

	auto newSize = newCount * sizeof(CPauseMenuItem);

	auto newItemArray = new CPauseMenuItem[newCount];

	memcpy_s(newItemArray, newSize, pMenu->items, pMenu->itemCount * sizeof(CPauseMenuItem));

	// overwrite the reset button since we will move it to the bottom later..
	auto itemIdx = pMenu->itemCount - 1;

	// hijack the last 50 settings indices with the hope that 
	// rockstar won't add more than 15 new settings in the future.. (max is 255 or CHAR_MAX)
	auto targetSettingIdx = kSettingsStartIndex;

	// add menu settings (kMenuItemsCount needs to be updated to reflect changes made here)

	auto languageId = getLanguageTextId();

	#pragma region FP Use Reticle

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYZ", 
		getConstString(languageId, MO_FP_USE_RETICLE), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "alwaysUseReticle";
		p.type = CPT_BOOLEAN;
		p.value.enabled = value != 0;

		setCamPreset(p);

	}, int(getPresetValueBool(eCamFirstPersonShooterCameraMetadata, "alwaysUseReticle", false)), 0)));

#pragma endregion

	#pragma region FP Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZY", 
		getConstString(languageId, MO_FP_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(-getPresetValueFloat(eCamFirstPersonShooterCameraMetadata, "minPitch", -60.0f), 40.0f, 80.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region FP Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYYY", 
		getConstString(languageId, MO_FP_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFirstPersonShooterCameraMetadata, "maxPitch", 60.0f), 40.0f, 80.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPP FOV

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXZ", 
		getConstString(languageId, MO_TP_FOV), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 20.0f, 80.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, "fov", 50.0f), 20.0f, 80.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPP Horizontal Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZ", 
		getConstString(languageId, MO_TP_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "pivotOffsetX";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.1f, 0.9f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, "pivotOffsetX", 0.4f), -0.1f, 0.9f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPP Follow Distance

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZY", 
		getConstString(languageId, MO_TP_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "followDistance";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0.0f, 2.0f);

		setCamPreset(p);
	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, "followDistance", 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPP Running Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYY", 
		getConstString(languageId, MO_TP_SHAKE), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "sprintShakeSpeed";
		p.type = CPT_FLOAT;
		p.value.fvalue = value != 0 ? 0.5f : FLT_MAX;

		setCamPreset(p);

	}, int(getPresetValueFloat(eCamFollowPedCameraMetadata, "sprintShakeSpeed", 0.5f) != FLT_MAX), 1)));

#pragma endregion

	#pragma region TPP Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXXX", 
		getConstString(languageId, MO_TP_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 50.0f, 90.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(-getPresetValueFloat(eCamFollowPedCameraMetadata, "minPitch", -70.0f), 50.0f, 90.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPP Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXY", 
		getConstString(languageId, MO_TP_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 1.0f, 89.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, "maxPitch", 45.0f), 0.0f, 90.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPA FOV

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYX", 
		getConstString(languageId, MO_TP_AIMING_FOV), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 15.0f, 75.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, "fov", 45.0f), 15.0f, 75.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPA Horizontal Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXY", 
		getConstString(languageId, MO_TP_AIMING_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
		p.name = "pivotOffsetX";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.1f, 0.9f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, "pivotOffsetX", 0.4f), -0.1f, 0.9f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPA Follow Distance

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYY", 
		getConstString(languageId, MO_TP_AIMING_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
		p.name = "followDistance";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0.0f, 2.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, "followDistance", 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region FPV FOV

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYY", 
		getConstString(languageId, MO_FP_VEHICLE_FOV), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, "fov", 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region FPV Vertical Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXX", 
		getConstString(languageId, MO_FP_VEHICLE_VERT_OFF), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "viewOffsetZ";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.05f, 0.05f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, "viewOffsetZ", 0.0f), -0.05f, 0.05f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region FPV Switch In Water

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZY", 
		getConstString(languageId, MO_FP_VEHICLE_SWITCH_WATER), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		if (value)
		{
			g_cinematicCameraEnterWaterPatch1.remove();
		}
		else
			g_cinematicCameraEnterWaterPatch1.install();

		g_scriptconfig.set<bool>("GlobalSettings", "SwapCameraOnWaterEnter", value != 0);

	},
		g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnWaterEnter", true), 1)));

#pragma endregion

	#pragma region FPV Switch On Desroyed

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXX", 
		getConstString(languageId, MO_FP_VEHICLE_SWITCH_DAMAGE), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		if (value)
		{
			g_cinematicCameraEnterWaterPatch2.remove();
		}
		else
			g_cinematicCameraEnterWaterPatch2.install();

		g_scriptconfig.set<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", value != 0);

	},
		g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", true), 1)));

#pragma endregion

	#pragma region FPV Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZX", 
		getConstString(languageId, MO_FP_VEHICLE_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, -30.0f, 50.0f);

		setCamPreset(p);

		p.name = "minPitchExt";
		setCamPreset(p);
	}, int(Math::FromToRange(-getPresetValueFloat(eCamCinematicMountedCameraMetadata, "minPitch", -10.0f), -30.0, 50.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region FPV Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYZX", 
		getConstString(languageId, MO_FP_VEHICLE_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -25.0f, 55.0f);

		setCamPreset(p);

		p.name = "maxPitchExt";
		setCamPreset(p);
	}, int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, "maxPitch", 15.0f), -25.0, 55.0, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPV Field Of View

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZZ", 
		getConstString(languageId, MO_TP_VEHICLE_FOV), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, "fov", 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPV Auto Center

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXXY", 
		getConstString(languageId, MO_TP_VEHICLE_AUTO_CENTER), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "enableAutoCenter";
		p.type = CPT_BOOLEAN;
		p.value.enabled = value != 0;

		setCamPreset(p);

	}, getPresetValueBool(eCamFollowVehicleCameraMetadata, "enableAutoCenter", true), 1)));

#pragma endregion

	#pragma region TPV Follow Distance

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZX", 
		getConstString(languageId, MO_TP_VEHICLE_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "followDistance";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.925f, 3.075f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, "followDistance", 1.075f), -0.925f, 3.075f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPV Follow Height

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXZ", 
		getConstString(languageId, MO_TP_VEHICLE_PIVOT_SCALE), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "pivotScale";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0, 2.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, "pivotScale", 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPV Horizontal Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXY", 
		getConstString(languageId, MO_TP_VEHICLE_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "pivotOffsetX";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.5f, 0.5f);

		setCamPreset(p);

	}, int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, "pivotOffsetX", 0.0f), -0.5f, 0.5f, 0.0f, 10.0f)), 5)));

#pragma endregion

	#pragma region TPV High Speed Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYZ", 
		getConstString(languageId, MO_TP_VEHICLE_SHAKE), 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "highSpeedShakeSpeed";
		p.type = CPT_FLOAT;
		p.value.fvalue = value != 0 ? 40.0f : FLT_MAX;

		setCamPreset(p);

	}, int(getPresetValueFloat(eCamFollowVehicleCameraMetadata, "highSpeedShakeSpeed", 40.0f) != FLT_MAX), 1)));

#pragma endregion

	// move reset button to the bottom
	newItemArray[itemIdx] = pMenu->items[pMenu->itemCount - 1];

	itemIdx++;

	LOG("addPauseMenuItems(): Expanding pause menu array...");

	pMenu->items = newItemArray;

	pMenu->itemCount = itemIdx;

	pMenu->maxItems = itemIdx;

	pMenu->scrollFlags &= 2; // add scroll bar

	LOG("addPauseMenuItems(): Finished adding menu items!");
}

bool SetMenuSlot_Stub(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bSlotUpdate)
{
	if (settingIndex >= kSettingsStartIndex)
	{
		auto it = g_customPrefs.find(settingIndex);

		if (it != g_customPrefs.end())
		{
			value = it->second.m_value;
		}
	}

	return g_createToggleItem->fn(columnId, slotIndex, menuState, settingIndex, unk, value, text, bPopScaleform, bSlotUpdate);
}

void SetPauseMenuPreference_Stub(long long settingIndex, int value, unsigned int unk)
{
	if (settingIndex >= kSettingsStartIndex)
	{
		auto prefId = int(settingIndex);

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

	g_setPauseMenuPreference->fn(settingIndex, value, unk);
}

void ResetCameraProfileSettings_Stub()
{
	g_resetCameraSettings->fn();

	LOG("Resetting custom prefs...");

	for (auto it = g_customPrefs.begin(); it != g_customPrefs.end(); ++it)
	{
		SetPauseMenuPreference_Stub(it->first, it->second.m_resetvalue, 3u);
	}

	saveCamPresets();
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
	if (!Utility::FileExists(g_scriptconfig.filename))
	{
		LOG("makeConfigFile(): Creating new config...");

		printToScreen("Creating config file...");

		std::string resText = getResourceConfigData(g_currentModule);

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

double getRemoteVersionNumber()
{
	TCHAR szFilename[MAX_PATH];

	auto hr = URLDownloadToCacheFileA(NULL, "http://www.camx.me/gtav/ecs/version.txt", szFilename, MAX_PATH, 0, NULL);

	if (SUCCEEDED(hr))
	{
		std::ifstream file(szFilename, std::ifstream::in);

		double fRemoteVersion;

		if (file.is_open())
		{
			file >> fRemoteVersion;

			file.close();

			remove(szFilename);

			return fRemoteVersion;
		}
	}

	return -1.0f;
}

void validateAppVersion()
{
	if (g_scriptconfig.get<bool>("General", "Notification", false))
	{
		auto fVersionNum = getRemoteVersionNumber();

		if (fVersionNum != -1.0 && fVersionNum > APP_VERSION)
		{
			printToScreen("A newer version is available (%.2f)", fVersionNum);
		}
	}
}

bool getMetadataHashForEntity(Entity entity, eMetadataHash eType, unsigned int * outHash)
{
	if (ENTITY::IS_ENTITY_A_VEHICLE(entity))
	{
		auto modelInfo = *reinterpret_cast<uintptr_t*>(getScriptHandleBaseAddress(entity) + 0x20);

		if (!modelInfo)
			return false;

		auto modelOffsets = g_addresses.get("CVehicleModelInfo");

		switch (eType)
		{
		case eCamCinematicMountedCameraMetadata:
			*outHash = *reinterpret_cast<DWORD*>(modelInfo + modelOffsets->map["firstPersonCameraHash"]);
			return true;

		case eCamFollowVehicleCameraMetadata:
			*outHash = *reinterpret_cast<DWORD*>(modelInfo + modelOffsets->map["thirdPersonCameraHash"]);
			return true;

		default:
			break;
		}
	}

	else if (ENTITY::IS_ENTITY_A_PED(entity))
	{
		switch (eType)
		{
		case eCamFirstPersonShooterCameraMetadata:
			*outHash = 0xA70102CA;
			break;
		case eCamFollowPedCameraMetadata:
			*outHash = 0xFBE36564;
			break;
		case eCamThirdPersonPedAimCameraMetadata:
			*outHash = 0xBB570E9E;
			break;
		default:
			return false;
		}

		return true;
	}

	return false;
}

void checkCameraFrame()
{
	Entity entity;

	auto playerPed = PLAYER::PLAYER_PED_ID();

	if (g_camGameplayDirector->vehicle)
	{
		entity = AI::GET_IS_TASK_ACTIVE(playerPed, 160) ?
			PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(playerPed) :
			PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
		bVehicleCamActive = true;
	}

	else
	{
		entity = playerPed;
		bVehicleCamActive = false;
	}

	unsigned int modelHash = ENTITY::GET_ENTITY_MODEL(entity);

	if (!modelHash || modelHash == g_modelId && !bShouldUpdatePresets) return;

	g_modelId = modelHash;

	bShouldUpdatePresets = false;

	auto keyValue = bUseGlobalPresets ? 0 : g_modelId;

	LOG("checkCameraFrame(): Looking up preset data using key %d...", keyValue);

	auto itPreset = g_camPresets.find(keyValue);

	if (itPreset == g_camPresets.end())
	{
		LOG("checkCameraFrame(): Preset lookup failed. Will try to use global preset.");

		itPreset = g_camPresets.find(0);

		if (itPreset == g_camPresets.end())
		{
			LOG("checkCameraFrame(): No global preset exists. Exiting...");
			return;
		}
	}

	LOG("checkCameraFrame(): Found camera presets. applying...");

	unsigned int cameraHash;

	for (auto it = itPreset->second.begin(); it != itPreset->second.end(); ++it)
	{
		auto addresses = g_addresses.get(g_metadataHashMap[it->metadataHash]);

		if (!addresses)
		{
			LOG("checkCameraFrame(): Couldn't find offsets for metadata (0x%lX)...", it->metadataHash);
			continue;
		}

		if (!getMetadataHashForEntity(entity, it->metadataHash, &cameraHash))
		{
			LOG("checkCameraFrame(): Couldn't find camera hash for entity. Base type was %s", g_metadataHashMap[it->metadataHash].c_str());
			continue;
		}

		auto camObjMetadata = getCamMetadataForHash(cameraHash);

		if (!camObjMetadata)
		{
			LOG("checkCameraFrame(): No metadata for hash 0x%lX.", camObjMetadata);
			continue;
		}

		auto psoData = camObjMetadata->getPsoStruct();

		if (!psoData) continue;

		auto baseMetadataHash = *reinterpret_cast<DWORD*>(psoData + 8);

		if (baseMetadataHash != it->metadataHash)
		{
			LOG("checkCameraFrame(): Found metadata type didn't match ours. Ours was 0x%lX but we found 0x%lX.", it->metadataHash, baseMetadataHash);
			continue;
		}

		auto address = reinterpret_cast<uintptr_t>(camObjMetadata) + addresses->map[it->name];

		switch (it->type)
		{
		case CPT_BOOLEAN:
			WriteBool(address, it->value.enabled);
			break;
		case CPT_INTEGER:
			WriteInt(address, it->value.integer);
			break;
		case CPT_UINTEGER:
			WriteUInt(address, it->value.unsignedInt);
			break;
		case CPT_FLOAT:
			WriteFloat(address, it->value.fvalue);
			break;
		case CPT_DOUBLE:
			WriteDouble(address, it->value.dvalue);
			break;

		default:
			break;
		}
	}
}

void scriptKeyboardMessage(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow)
{
	if (key == vkSaveLayout && GetAsyncKeyState(VK_CONTROL) & 0x8000)
	{
		saveCamPresets();

		bShouldPlaySound = true;
	}

	else if (key == vkReloadPresets)
	{
		printToScreen("Reloading camera settings");

		readPresetsFromFile("CameraPresets.xml");

		bShouldUpdatePresets = true;
	}	
}

void updateFrontendSound()
{
	if (bShouldPlaySound)
	{
		if (GAMEPLAY::GET_GAME_TIMER() - lastSoundPlayedTime > 2000)
		{
			AUDIO::PLAY_SOUND_FRONTEND(-1, "OTHER_TEXT", "HUD_AWARDS", 1);

			lastSoundPlayedTime = GAMEPLAY::GET_GAME_TIMER();
		}
		bShouldPlaySound = false;
	}
}

void mainLoop()
{
	while (true)
	{
		WAIT(0);

		checkCameraFrame();

		updateFrontendSound();

		if (bInitialized || *g_gameState != Playing) continue;

		if (!g_scriptconfig.get<bool>("GlobalSettings", "GamepadFollowCamAutoCenter", true))
		{
			g_autoRotatePatch.install();
		}

		if (!g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnWaterEnter", true))
		{
			g_cinematicCameraEnterWaterPatch1.install();
		}

		if (!g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", true))
		{
			g_cinematicCameraEnterWaterPatch2.install();
		}

		validateAppVersion();

		printToScreen("Loaded");

		bInitialized = true;
	}
}

void setupHooks()
{
	auto gameVersion = getGameVersion();

	// invalid game version
	if (gameVersion == VER_UNK) return;

	addOffsetsForGameVersion(gameVersion);

	// get pointer to game state..
	auto result = Pattern((BYTE*)"\x0F\x29\x74\x24\x00\x85\xDB", "xxxx?xx").get();

	if (result)
	{
		LOG("main(): g_gameState found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_gameState");
		return;
	}

	g_gameState = reinterpret_cast<eGameState*>(*reinterpret_cast<int *>(result - 4) + result);

	// jmp patch #1 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x31\x81\x00\x00\x00\x00\xF3\x0F\x10\x44\x24\x00", "xx????xxxxx?").get(76);

	if (result)
	{
		LOG("main(): g_enterWaterPatch1 found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_enterWaterPatch1");
		return;
	}

	g_cinematicCameraEnterWaterPatch1 = bytepatch_t((BYTE*)result, (gameVersion < v1_0_877_1_STEAM && gameVersion > v1_0_505_2_NOSTEAM) ?
		std::vector<BYTE>(6, NOP) : gameVersion < v1_0_944_2_STEAM ? std::vector<BYTE> { JMPREL_8 } : std::vector<BYTE>(6, NOP)); // jz = jmp		
																																  // jmp patch #2 for stop camera swap on vehicle enter water
	result = Pattern((BYTE*)"\x44\x8A\xC5\x48\x8B\x0C\xC8", "xxxxxxx").get(48);

	if (result)
	{
		LOG("main(): g_enterWaterPatch2 found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_enterWaterPatch2");
		return;
	}

	g_cinematicCameraEnterWaterPatch2 = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP)); // jz = nop

	result = Pattern((BYTE*)"\xF3\x0F\x10\x07\x0F\x28\xCF", "xxxxxxx").get(-23);

	if (result)
	{
		LOG("main(): g_autoRotatePatch found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_autoRotatePatch");
		return;
	}

	g_autoRotatePatch = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP));

	result = Pattern((BYTE*)"\x88\x50\x41\x48\x8B\x47\x40", "xxxxxxx").get(-0x28);

	if (result)
	{
		LOG("main(): g_metadataCollection found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_metadataCollection");
		return;
	}

	result = *reinterpret_cast<int *>(result - 4) + result + 6;

	g_metadataCollection = reinterpret_cast<rage::pgCollection<camMetadataRef*>*>((*reinterpret_cast<int *>(result + 3) + result - 1));

	result = Pattern((BYTE*)"\x0F\xB7\x54\x51\x00", "xxxx?").get();

	if (result)
	{
		LOG("main(): g_activeMenuArray found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_activeMenuArray");
		return;
	}

	g_activeMenuArray = *reinterpret_cast<rage::pgCollection<CPauseMenuInstance>*>(*reinterpret_cast<int *>(result - 4) + result);

	result = Pattern((BYTE*)"\x48\x85\xC0\x75\x34\x8B\x0D", "xxxxxxx").get(-0x5);

	if (result)
	{
		LOG("main(): getGxtEntry() found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find getGxtEntry()");
		return;
	}

	g_getGxtEntry = HookManager::SetCall<GetGlobalTextEntry_t>(result, getGxtEntry_Stub);

	Pattern pattern = Pattern((BYTE*)"\x83\xFF\x05\x74\x15", "xxxxx");

	result = pattern.get(-0x1A);

	if (result)
	{
		LOG("main(): SetMenuSlot() #1 found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find setMenuSlot() #1");
		return;
	}

	g_createSliderItem = HookManager::SetCall<SetMenuSlot_t>(result, SetMenuSlot_Stub); //-0x1A

	result = pattern.get(0xA8);

	if (result)
	{
		LOG("main(): SetMenuSlot() #2 found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find SetMenuSlot() #2");
		return;
	}

	g_createToggleItem = HookManager::SetCall<SetMenuSlot_t>(result, SetMenuSlot_Stub); // +0xA8

	pattern = Pattern((BYTE*)"\x81\xE9\x00\x00\x00\x00\x74\x25\xFF\xC9", "xx????xxxx");

	result = pattern.get(-0x2A);

	if (result)
	{
		LOG("main(): ResetCameraProfileSettings() found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find ResetCameraProfileSettings()");
		return;
	}

	g_resetCameraSettings = HookManager::SetCall<ResetCameraProfileSettings_t>(result, ResetCameraProfileSettings_Stub);

	pattern = Pattern((BYTE*)"\xF2\x0F\x2C\x56\x00", "xxxx?");

	result = pattern.get(0x20);

	if (result)
	{
		LOG("main(): setPauseMenuPreference() found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find SetPauseMenuPreference()");
		return;
	}

	g_setPauseMenuPreference = HookManager::SetCall<SetPauseMenuPreference_t>(result, SetPauseMenuPreference_Stub);

	result = pattern.get(0x12);

	// always toggle preferences
	memset((void*)result, 0x90, 6);

	pattern = Pattern((BYTE*)"\x39\x18\x74\x0A\x48\xFF\xC6", "xxxxxxx");

	result = pattern.get(-0x48);

	if (result)
	{
		LOG("main(): getCamDirectorPoolObject() found at 0x%llX", result);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find getCamDirectorPoolObject()");
		return;
	}

	getCamDirectorFromPool = reinterpret_cast<GetCamDirectorFromPool_t>(result);

	auto hashKey = getHashKey("camgameplaydirector");

	g_camGameplayDirector = getCamDirectorFromPool(&hashKey);

	if (g_camGameplayDirector)
	{
		LOG("main(): g_camGameplayDirector found at %p", g_camGameplayDirector);
	}

	else
	{
		LOG("[ERROR] main(): Failed to find g_camGameplayDirector");
	}
}

void loadConfigData()
{
	char inBuf[MAX_PATH];

	if (g_scriptconfig.getText(inBuf, "Keybinds", "SaveSettings"))
	{
		vkSaveLayout = keyFromString(std::string(inBuf), 0x42);
	}

	if (g_scriptconfig.getText(inBuf, "Keybinds", "ReloadSettings"))
	{
		vkReloadPresets = keyFromString(std::string(inBuf), VK_F11);
	}

	if (!g_scriptconfig.get<bool>("General", "UseCustomPresets", true))
	{
		bUseGlobalPresets = true;
	}

	if (g_scriptconfig.get<bool>("General", "AutoSaveLayouts", false))
	{
		bAutoSaveLayouts = true;
	}
}

void scriptMain()
{
	// script is being reloaded or something.. skip to loop
	if (bDidLoad)
	{
		mainLoop();
	}

	else
	{
		readPresetsFromFile("CameraPresets.xml");

		makeConfigFile();

		loadConfigData();

		setupHooks();

		checkCameraFrame(); // pump camera frame for addPauseMenuItems();

		addPauseMenuItems();

		bDidLoad = true;
	}
	
	// begin infinite loop...
	mainLoop();
}

void mainInit(HMODULE hModule)
{
	g_currentModule = hModule;
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

	pMenu->scrollFlags |= 48;

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

	if (g_autoRotatePatch.active)
	{
		g_autoRotatePatch.remove();
	}
}

void removeHooks()
{
	if (g_getGxtEntry)
	{
		delete g_getGxtEntry;
		g_getGxtEntry = NULL;
	}

	if (g_setPauseMenuPreference)
	{
		delete g_setPauseMenuPreference;
		g_setPauseMenuPreference = NULL;
	}

	if (g_createSliderItem)
	{
		delete g_createSliderItem;
		g_createSliderItem = NULL;
	}

	if (g_createToggleItem)
	{
		delete g_createToggleItem;
		g_createToggleItem = NULL;
	}

	if (g_resetCameraSettings)
	{
		delete g_resetCameraSettings;
		g_resetCameraSettings = NULL;
	}
}

void unload()
{
	LOG("Unloading menu items...");

	removePauseMenuItems();

	LOG("Unloading patches...");

	removePatches();

	LOG("Remove hooks...");

	removeHooks();

	g_textEntries.clear();

	g_customPrefs.clear();
}
