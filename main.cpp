
#include "stdafx.h"

#pragma warning(disable : 4244 4305) 

typedef const char *(*GetGlobalTextEntry_t)(const char * text, unsigned int hashName);

typedef void(*SetPauseMenuPreference_t)(long long settingIndex, int value, unsigned int unk);

typedef bool(*SetMenuSlot_t)(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char * text, bool bPopScaleform, bool bIsSlotUpdate);

typedef void(*ResetCameraProfileSettings_t)();

typedef camBaseDirector* (*GetCamDirectorObjectByHash_t)(unsigned int * hashName);

HMODULE g_currentModule;

AddressMgr g_addresses;

eGameState * g_gameState;

bytepatch_t g_cinematicCameraEnterWaterPatch1,
g_cinematicCameraEnterWaterPatch2;

CallHook<GetGlobalTextEntry_t> * g_getGxtEntry;

CallHook<SetPauseMenuPreference_t> * g_setPauseMenuPreference;

CallHook<SetMenuSlot_t> * g_createSliderItem;

CallHook<SetMenuSlot_t> * g_createToggleItem;

CallHook<ResetCameraProfileSettings_t> * g_resetCameraSettings;

rage::pgCollection<camBaseDirector*> * g_camDirectorPool;

rage::pgCollection<camMetadataRef*> * g_metadataCollection;

rage::pgCollection<CPauseMenuInstance> g_activeMenuArray;

GetCamDirectorObjectByHash_t getCamDirectorObjectByHash;

camBaseCamera * g_activeCamera;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

CConfig g_scriptconfig = CConfig("ExtendedCameraSettings.ini");

const int kMenuItemsCount = 19;

bool bInitialized = false;

bool bUseCameraIndependentSettings = false;

std::map<unsigned int, std::vector<CamMetadataPreset>> g_camPresets;

std::map<int, CustomMenuPref> g_customPrefs;

unsigned int g_modelId;

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
};

void addOffsetsForGameVersion(int gameVer)
{
#pragma region firstPersonCamera

	auto offsets = g_addresses.getOrCreate("camFirstPersonShooterCameraMetadata");

	offsets->insert("fov", 36);
	offsets->insert("minPitch", 84);
	offsets->insert("maxPitch", 88);
	offsets->insert("altMinYaw", gameVer < v1_0_505_2_STEAM ? 696 : gameVer < v1_0_877_1_STEAM ? 712 : 760);
	offsets->insert("altMaxYaw", gameVer < v1_0_505_2_STEAM ? 700 : gameVer < v1_0_877_1_STEAM ? 716 : 764);
	offsets->insert("altMinPitch", gameVer < v1_0_505_2_STEAM ? 704 : gameVer < v1_0_877_1_STEAM ? 720 : 768);
	offsets->insert("altMaxPitch", gameVer < v1_0_505_2_STEAM ? 708 : gameVer < v1_0_877_1_STEAM ? 724 : 772);
	offsets->insert("alwaysUseReticle", 114);
	offsets->insert("relativeOffsetX", gameVer < v1_0_505_2_STEAM ? 68 : 64);
	offsets->insert("relativeOffsetY", offsets->map["relativeOffsetX"].add(4));
	offsets->insert("relativeOffsetZ", offsets->map["relativeOffsetX"].add(8));

#pragma endregion

#pragma region cinematicMountedCamera

	offsets = g_addresses.getOrCreate("camCinematicMountedCameraMetadata");

	offsets->insert("fov", gameVer < v1_0_877_1_STEAM ? 80 : 84);
	offsets->insert("minPitch", gameVer < v1_0_505_2_STEAM ? 808 : gameVer < v1_0_877_1_STEAM ? 824 : gameVer < v1_0_944_2_STEAM ? 872 : 888);
	offsets->insert("maxPitch", gameVer < v1_0_505_2_STEAM ? 812 : gameVer < v1_0_877_1_STEAM ? 828u : gameVer < v1_0_944_2_STEAM ? 876 : 892);
	offsets->insert("minPitchExtended", gameVer < v1_0_505_2_STEAM ? 776 : gameVer < v1_0_877_1_STEAM ? 792 : gameVer < v1_0_944_2_STEAM ? 840 : 856);
	offsets->insert("maxPitchExtended", gameVer < v1_0_505_2_STEAM ? 780 : gameVer < v1_0_877_1_STEAM ? 796 : gameVer < v1_0_944_2_STEAM ? 844 : 860);
	offsets->insert("minSpeedForCorrect", gameVer < v1_0_505_2_STEAM ? 680 : gameVer < v1_0_877_1_STEAM ? 696 : gameVer < v1_0_944_2_STEAM ? 744 : 760);
	offsets->insert("relativeOffsetX", gameVer < v1_0_877_1_STEAM ? 80 : 96);
	offsets->insert("relativeOffsetY", offsets->map["relativeOffsetX"].add(4));
	offsets->insert("relativeOffsetZ", offsets->map["relativeOffsetX"].add(8));

#pragma endregion

#pragma region followVehicleCamera

	offsets = g_addresses.getOrCreate("camFollowVehicleCameraMetadata");

	offsets->insert("fov", 48);
	offsets->insert("highSpeedShakeSpeed", gameVer < v1_0_505_2_STEAM ? 1176 : gameVer < v1_0_944_2_STEAM ? 1192 : 1208);
	offsets->insert("enableAutoCenter", gameVer < v1_0_505_2_STEAM ? 877 : gameVer < v1_0_944_2_STEAM ? 893 : 909); //F3 0F 10 8B ? ? ? ? F3 0F 11 44 24 ? F3 0F 10 83 ? ? ? ? F3 0F 5C CA
	offsets->insert("autoCenterLerpScale", gameVer < v1_0_505_2_STEAM ? 892 : gameVer < v1_0_944_2_STEAM ? 908 : 924); //1011 //F3 0F 10 88 ? ? ? ? 73 06 
	offsets->insert("followDistance", gameVer < v1_0_791_2_STEAM ? 312 : gameVer < v1_0_944_2_STEAM ? 320 : 328); // 350 312
	offsets->insert("pivotScale", gameVer < v1_0_791_2_STEAM ? 164 : gameVer < v1_0_944_2_STEAM ? 172 : 180);
	offsets->insert("pivotOffsetX", gameVer < v1_0_791_2_STEAM ? 232 : gameVer < v1_0_944_2_STEAM ? 240 : gameVer < v1_0_1011_1_STEAM ? 256 : 248); //48 8B 81 ? ? ? ? F3 0F 10 B0 ? ? ? ? F3 0F 10 B8 ? ? ? ? 48 8B 05 ? ? ? ? 
	offsets->insert("pivotOffsetY", offsets->map["pivotOffsetX"].add(4));
	offsets->insert("pivotOffsetZ", offsets->map["pivotOffsetX"].add(8));

#pragma endregion

#pragma region followPedCamera

	offsets = g_addresses.getOrCreate("camFollowPedCameraMetadata");

	offsets->insert("sprintShakeSpeed", gameVer < v1_0_505_2_STEAM ? 2068 : 2092);
	offsets->insert("minPitch", gameVer < v1_0_505_2_STEAM ? 580 : gameVer < v1_0_944_2_STEAM ? 596 : 612);
	offsets->insert("maxPitch", gameVer < v1_0_505_2_STEAM ? 584 : gameVer < v1_0_944_2_STEAM ? 600 : 616);

#pragma endregion
}

std::string getPresetValueString(CamMetadataPreset preset)
{
	std::stringstream sstream;

	switch (preset.type)
	{
	case CPT_BOOLEAN:
		sstream << std::boolalpha << preset.value.enabled;
		break;
	case CPT_INTEGER:
		sstream << preset.value.integer;
		break;
	case CPT_UINTEGER:
		sstream << preset.value.unsignedInt;
		break;
	case CPT_FLOAT:
		sstream << preset.value.fvalue;
		break;
	case CPT_DOUBLE:
		sstream << preset.value.dvalue;
		break;
	default:
		break;
	}

	return sstream.str();
}

void readXmlDataFileEntries(std::string filename)
{
	if (!Utility::FileExists(filename))
	{
		LOG("readXmlDataFileEntries(): File not found (%s)", filename.c_str());
		return;
	}

	tinyxml2::XMLDocument doc;

	auto filePath = Utility::GetWorkingDirectory() + "\\" + filename;

	auto result = doc.LoadFile(filePath.c_str());

	if (result != tinyxml2::XML_SUCCESS)
	{
		LOG("readXmlDataFileEntries(): Encountered an error loading the file %s (%d)", filePath.c_str(), result);
		return;
	}

	auto rootElement = doc.FirstChildElement("cameraPresets");

	if (!rootElement)
	{
		LOG("writeXmlDataFileEntries(): No root element.");
		return;
	}

	XMLHelper::ForEach(rootElement, "camPreset", [](XMLElement* e) {

		auto modelName = e->FirstChildElement("modelName")->GetText();

		std::vector<CamMetadataPreset> presets;

		for (auto it = g_metadataHashMap.begin(); it != g_metadataHashMap.end(); it++)
		{
			auto element = XMLHelper::FirstElement(e, it->second);

			if (!element) continue;

			LOG("readXmlDataFileEntries(): Adding %s presets for model %s", it->second.c_str(), modelName);

			for (auto p = element->FirstChildElement("Preset"); p != nullptr; p = p->NextSiblingElement("Preset"))
			{
				CamMetadataPreset preset;

				preset.name = p->Attribute("name");

				preset.metadataHash = (eMetadataHash)it->first;

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

				LOG("Added new preset with type: %s value: %s", type, value);
			}
		}

		g_camPresets.insert(std::make_pair(parseHashString(modelName), presets));
	});
}

void writePresetsToFile(std::string filename, unsigned int modelHash, std::vector<CamMetadataPreset> presets)
{
	if (!Utility::FileExists(filename))
	{
		LOG("writeXmlDataFileEntries(): File not found (%s)", filename.c_str());
		return;
	}

	tinyxml2::XMLDocument doc;

	auto filePath = Utility::GetWorkingDirectory() + "\\" + filename;

	auto result = doc.LoadFile(filePath.c_str());

	if (result != tinyxml2::XML_SUCCESS)
	{
		LOG("writeXmlDataFileEntries(): Encountered an error loading the file %s (%d)", filePath.c_str(), result);
		return;
	}

	auto rootElement = doc.FirstChildElement("cameraPresets");

	if (!rootElement)
	{
		LOG("writeXmlDataFileEntries(): No root element.");
		return;
	}

	auto elem = XMLHelper::FindIf(rootElement, "camPreset", [modelHash](XMLElement* e) -> bool {
		auto str = e->FirstChildElement("modelName")->GetText();
		return modelHash == parseHashString(str);
	});

	if (!elem)
	{
		LOG("writeXmlDataFileEntries(): No entry found for hash. Create a new one...");

		elem = doc.NewElement("camPreset");

		auto subNode = doc.NewElement("modelName");

		subNode->SetText(std::to_string(modelHash).c_str());

		elem->LinkEndChild(subNode);

		rootElement->LinkEndChild(elem);
	}

	LOG("writeXmlDataFileEntries(): Setting presets...");

	for (auto it = presets.begin(); it != presets.end(); it++)
	{
		auto nameStr = g_metadataHashMap[it->metadataHash];

		auto childElem = XMLHelper::FirstElement(elem, nameStr);

		if (!childElem)
		{
			LOG("writeXmlDataFileEntries(): Element contains no %s node. Creating a new child node...", nameStr.c_str());

			childElem = doc.NewElement(nameStr.c_str());

			elem->LinkEndChild(childElem);
		}

		auto node = XMLHelper::FindByAttribute(childElem, "Preset", "name", it->name);

		if (node)
		{
			LOG("writeXmlDataFileEntries(): Preset node exists. setting value...");

			node->SetText(it->toString().c_str());

			continue;
		}

		else 
			LOG("writeXmlDataFileEntries(): No preset node found. Creating a new one...");

		node = doc.NewElement("Preset");

		node->SetAttribute("name", it->name.c_str());

		node->SetAttribute("type", g_dataFileTypeMap[it->type].c_str());

		node->SetText(it->toString().c_str());

		childElem->LinkEndChild(node);
	}

	doc.SaveFile(filename.c_str());
}

void writePresetToFile(std::string filename,unsigned int modelHash, CamMetadataPreset preset)
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
		if (it && it->menuId == menuIndex)
		{
			return it;
		}
	}

	return nullptr;
}

void patchFirstPersonShooterCameraMetadata(MemAddr baseAddress)
{
	auto metadataName = g_metadataHashMap[eCamFirstPersonShooterCameraMetadata];

	auto& addresses = (*g_addresses.get(metadataName));

	WriteFloat(baseAddress.add(addresses["fov"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "FOV", 45.0f));

	WriteFloat(baseAddress.add(addresses["minPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "MinPitch", -80.0f));

	WriteFloat(baseAddress.add(addresses["maxPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "MaxPitch", 80.0f));

	WriteFloat(baseAddress.add(addresses["altMinPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "AltMinPitch", -75.0f));

	WriteFloat(baseAddress.add(addresses["altMaxPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "AltMaxPitch", -75.0f));

	WriteFloat(baseAddress.add(addresses["altMinYaw"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "AltMinYaw", -45.0f));

	WriteFloat(baseAddress.add(addresses["altMaxYaw"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "AltMaxYaw", 45.0f));

	WriteBool(baseAddress.add(addresses["alwaysUseReticle"]).addr, g_scriptconfig.get<bool>(metadataName.c_str(), "AlwaysUseReticle", false));

	WriteFloat(baseAddress.add(addresses["relativeOffsetX"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetX", 0.0f));

	WriteFloat(baseAddress.add(addresses["relativeOffsetY"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetY", 0.0f));

	WriteFloat(baseAddress.add(addresses["relativeOffsetZ"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetZ", 0.0f));
}

void patchCinematicMountedCameraMetadata(MemAddr baseAddress)
{
	auto metadataName = g_metadataHashMap[eCamCinematicMountedCameraMetadata];

	auto& addresses = (*g_addresses.get(metadataName));

	auto fPitchValue = g_scriptconfig.get<float>(metadataName.c_str(), "MinPitch", -10.0f);

	WriteFloat(baseAddress.add(addresses["minPitch"]).addr, fPitchValue);

	WriteFloat(baseAddress.add(addresses["minPitchExtended"]).addr, fPitchValue);

	fPitchValue = g_scriptconfig.get<float>(metadataName.c_str(), "MaxPitch", 15.0f);

	WriteFloat(baseAddress.add(addresses["maxPitch"]).addr, fPitchValue);

	WriteFloat(baseAddress.add(addresses["maxPitchExtended"]).addr, fPitchValue);

	WriteFloat(baseAddress.add(addresses["fov"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "FOV", 50.0f));

	WriteFloat(baseAddress.add(addresses["relativeOffsetX"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetX", 0.0f));

	WriteFloat(baseAddress.add(addresses["relativeOffsetY"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetY", 0.0f));

	WriteFloat(baseAddress.add(addresses["relativeOffsetZ"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "RelativeOffsetZ", 0.0f));

	WriteFloat(baseAddress.add(addresses["minSpeedForCorrect"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "MinSpeedForCorrect", 20.0f));
}

void patchFollowVehicleCameraMetadata(MemAddr baseAddress)
{
	auto metadataName = g_metadataHashMap[eCamFollowVehicleCameraMetadata];

	auto& addresses = (*g_addresses.get(metadataName));

	WriteFloat(baseAddress.add(addresses["fov"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "FOV", 50.0f));

	WriteFloat(baseAddress.add(addresses["pivotOffsetX"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "PivotOffsetX", 0.0f));

	WriteFloat(baseAddress.add(addresses["pivotOffsetY"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "PivotOffsetY", 0.0f));

	WriteFloat(baseAddress.add(addresses["pivotOffsetZ"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "PivotOffsetZ", 0.0f));

	WriteFloat(baseAddress.add(addresses["highSpeedShakeSpeed"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "HighSpeedShakeSpeed", 40.0f));

	WriteBool(baseAddress.add(addresses["enableAutoCenter"]).addr, g_scriptconfig.get<bool>(metadataName.c_str(), "EnableAutoCenter", false));

	WriteFloat(baseAddress.add(addresses["followDistance"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "FollowDistance", 1.0f));

	WriteFloat(baseAddress.add(addresses["pivotScale"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "PivotScale", 1.075f));
}

void patchFollowPedCameraMetadata(MemAddr baseAddress)
{
	auto metadataName = g_metadataHashMap[eCamFollowPedCameraMetadata];

	auto& addresses = (*g_addresses.get(metadataName));

	WriteFloat(baseAddress.add(addresses["sprintShakeSpeed"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "SprintShakeSpeed", 0.5f));

	WriteFloat(baseAddress.add(addresses["minPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "MinPitch", -70.0f));

	WriteFloat(baseAddress.add(addresses["maxPitch"]).addr, g_scriptconfig.get<float>(metadataName.c_str(), "MaxPitch", 45.0f));
}

template <typename T>
T patchMetadataValue(eMetadataHash type, std::string key, T value)
{
	auto addresses = g_addresses.get(g_metadataHashMap[type]);

	if (!addresses) return NULL;

	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (auto ref = *it; ref; ref = ref->pNext)
		{
			auto metadata = ref->pData;

			if (!metadata || !*reinterpret_cast<uintptr_t**>(metadata)) continue;

			if (ref->nameHash == 2648018977) continue; // jet mounted camera

			auto psoStruct = metadata->getPsoStruct();

			if (*reinterpret_cast<DWORD*>(psoStruct + 8) != type) continue;

			*reinterpret_cast<T*>(reinterpret_cast<uintptr_t>(metadata) + addresses->map[key]) = value;
		}
	}

	return value;
}

template <typename T>
bool patchMetadataValues(eMetadataHash type, std::string format, const std::vector<T> & args)
{
	if (format.empty() || args.empty()) return false;

	std::vector<std::string> parameters;

	Utility::SplitString(format, ";", parameters);

	if (parameters.size() != args.size()) return false;

	auto addresses = g_addresses.get(g_metadataHashMap[type]);

	if (!addresses) return false;

	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (auto ref = *it; ref; ref = ref->pNext)
		{
			auto metadata = ref->pData;

			if (!metadata || !*reinterpret_cast<uintptr_t**>(metadata)) continue;

			auto psoStruct = metadata->getPsoStruct();

			if (!psoStruct) continue;

			if (*reinterpret_cast<DWORD*>(psoStruct + 8) != type || ref->nameHash == 2648018977) continue;

			for (size_t i = 0; i < parameters.size(); i++)
			{
				*reinterpret_cast<T*>(
					reinterpret_cast<uintptr_t>(metadata) + addresses->map[parameters[i]]) = args[i];
			}
		}
	}

	return true;
}

camBaseObjectMetadata * getMetadataForHash(unsigned int hashName)
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

void patchMetadataCollection()
{
	LOG("patchMetadataCollection(): Begin patching metadata collection...");

	LOG("patchMetadataCollection(): %d metadata entries found.", g_metadataCollection->m_count);

	for (auto it = g_metadataCollection->begin(); it != g_metadataCollection->end(); it++)
	{
		if (!it) continue;

		for (auto ref = *it; ref; ref = ref->pNext)
		{
			auto metadata = ref->pData;

			if (metadata && *reinterpret_cast<uintptr_t**>(metadata))
			{
				if (ref->nameHash == 2648018977) continue; // jet mounted camera

				auto psoStruct = metadata->getPsoStruct();

				if (!psoStruct) continue;

				auto metadataTypeHash = *reinterpret_cast<DWORD*>(psoStruct + 8);

				auto address = MemAddr(metadata);

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

	LOG("patchMetadataCollection(): Finished patching with no error.");
}

void setupMenuItem(CPauseMenuItem * item, std::string gxtAlias, std::string text, int menuIndex, int type, int actionType, int settingIdx, int stateFlags)
{
	memset(item, 0x00, sizeof(CPauseMenuItem));

	item->textHash = addGxtEntry(gxtAlias, text);
	item->menuIndex = menuIndex;
	item->type = type;
	item->actionType = actionType;
	item->targetSettingIdx = static_cast<unsigned char>(settingIdx);
	item->stateFlags = stateFlags;

	LOG("setupMenuItem(): Populating item with type '%d' action '%d' GXT alias \"%s\" display name \"%s\"", type, actionType, gxtAlias.c_str(), text.c_str());
}

void setCamPreset(CamMetadataPreset& preset)
{
	if (bUseCameraIndependentSettings)
	{
		auto presets = g_camPresets[g_modelId];

		auto it = std::find_if(presets.begin(), presets.end(), 
			[&](const CamMetadataPreset&v) { 
			return preset.metadataHash == v.metadataHash && preset.name == v.name; });

		if (it != presets.end())
		{
			*it = preset;
		}

		else 
			g_camPresets[g_modelId].push_back(preset);

		writePresetToFile("CameraPresets.xml", g_modelId,  preset);
	}

	else
	{
		g_scriptconfig.set<const char*>(g_metadataHashMap[preset.metadataHash].c_str(), 
			preset.name.c_str(), 
			preset.toString().c_str());
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
	auto targetSettingIdx = 237;

	// add menu settings (kMenuItemsCount needs to be updated to reflect changes made here)

#pragma region FP Use Reticle

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYZ", "First Person Always Use Reticle", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "alwaysUseReticle";
		p.type = CPT_BOOLEAN;
		p.value.enabled = value != 0;

		setCamPreset(p);

	}, g_scriptconfig.get<bool>("eCamFirstPersonShooterCameraMetadata", "AlwaysUseReticle", false), 0)));

#pragma endregion

#pragma region FP Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZY", "First Person Min Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
	//	patchMetadataValue<float>(eCamFirstPersonShooterCameraMetadata, "minPitch", value);

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(-g_scriptconfig.get<float>("eCamFirstPersonShooterCameraMetadata", "MinPitch", -60.0f), 40.0f, 80.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region FP Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYYY", "First Person Max Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFirstPersonShooterCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("eCamFirstPersonShooterCameraMetadata", "MaxPitch", 60.0f), 40.0f, 80.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPP Running Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYY", "Third Person Running Shake", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "sprintShakeSpeed";
		p.type = CPT_FLOAT;
		p.value.fvalue = value != 0 ? 0.5f : FLT_MAX;

		setCamPreset(p);

	}, int(g_scriptconfig.get<float>("camFollowPedCameraMetadata", "sprintShakeSpeed", 0.5) != FLT_MAX), 1)));

#pragma endregion

#pragma region TPP Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXXX", "Third Person Min Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 50.0f, 90.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(-g_scriptconfig.get<float>("camFollowPedCameraMetadata", "MinPitch", -70.0f), 50.0f, 90.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPP Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXY", "Third Person Max Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowPedCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 1.0f, 89.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camFollowPedCameraMetadata", "MaxPitch", 45.0f), 1.0f, 89.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region FPV FOV

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXYY", "First Person Vehicle Field of View", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camCinematicMountedCameraMetadata", "FOV", 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region FPV Vertical Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYXX", "First Person Vehicle Vertical Origin", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "relativeOffsetZ";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.05f, 0.05f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camCinematicMountedCameraMetadata", "RelativeOffsetZ", 0.0f), -0.05f, 0.05f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region FPV Switch In Water

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZY", "First Person Vehicle Switch In Water", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		value ? g_cinematicCameraEnterWaterPatch1.remove() : g_cinematicCameraEnterWaterPatch1.install();

		g_scriptconfig.set<bool>("camCinematicMountedCameraMetadata", "SwapCameraOnWaterEnter", value != 0);

	}, g_scriptconfig.get<bool>("camCinematicMountedCameraMetadata", "SwapCameraOnWaterEnter", true), 1)));

#pragma endregion

#pragma region FPV Switch On Desroyed

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXX", "First Person Vehicle Switch on Destroyed", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {
		value ? g_cinematicCameraEnterWaterPatch2.remove() : g_cinematicCameraEnterWaterPatch2.install();

		g_scriptconfig.set<bool>("camCinematicMountedCameraMetadata", "SwapCameraOnVehicleDestroyed", value != 0);

	}, g_scriptconfig.get<bool>("camCinematicMountedCameraMetadata", "SwapCameraOnVehicleDestroyed", true), 1)));

#pragma endregion

#pragma region FPV Min Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZZX", "First Person Vehicle Min Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "minPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -25.0f, 55.0f);

		setCamPreset(p);

		p.name = "minPitchExtended";
		setCamPreset(p);
	}, int(Math::FromToRange(-g_scriptconfig.get<float>("camCinematicMountedCameraMetadata", "MinPitch", -15.0f), -25.0, 55.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region FPV Max Pitch Limit

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMYZX", "First Person Vehicle Max Pitch Angle", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamCinematicMountedCameraMetadata;
		p.name = "maxPitch";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -25.0f, 55.0f);

		setCamPreset(p);

		p.name = "maxPitchExtended";
		setCamPreset(p);
	}, int(Math::FromToRange(g_scriptconfig.get<float>("camCinematicMountedCameraMetadata", "MaxPitch", 15.0f), -25.0, 55.0, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPV Field Of Vieww

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZZ", "Third Person Vehicle Field of View", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "fov";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camFollowVehicleCameraMetadata", "FOV", 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPV Auto Center

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXXY", "Third Person Vehicle Auto Center", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {


		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "enableAutoCenter";
		p.type = CPT_BOOLEAN;
		p.value.enabled = value != 0;

		setCamPreset(p);

	}, g_scriptconfig.get<bool>("camFollowVehicleCameraMetadata", "EnableAutoCenter", false), 1)));

#pragma endregion

#pragma region TPV Follow Distance

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMXZX", "Third Person Vehicle Follow Distance", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "followDistance";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.925f, 3.075f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camFollowVehicleCameraMetadata", "FollowDistance", 1.075f), -0.925f, 3.075f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPV Follow Height

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXZ", "Third Person Vehicle Pivot Scale", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "pivotScale";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0, 2.0f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camFollowVehicleCameraMetadata", "PivotScale", 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPV Horizontal Origin

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZXY", "Third Person Vehicle Horizontal Origin", 51, 2, Slider, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "pivotOffsetX";
		p.type = CPT_FLOAT;
		p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.5f, 0.5f);

		setCamPreset(p);

	}, int(Math::FromToRange(g_scriptconfig.get<float>("camFollowVehicleCameraMetadata", "PivotOffsetX", 0.0f), -0.5f, 0.5f, 0.0f, 10.0f)), 5)));

#pragma endregion

#pragma region TPV High Speed Shake

	setupMenuItem(&newItemArray[itemIdx++], "MO_CUSTOMZYZ", "Third Person Vehicle High Speed Shake", 51, 2, Toggle, targetSettingIdx, 0);

	g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

		CamMetadataPreset p;
		p.metadataHash = eCamFollowVehicleCameraMetadata;
		p.name = "highSpeedShakeSpeed";
		p.type = CPT_FLOAT;
		p.value.fvalue = value != 0 ? 40.0f : FLT_MAX;

		setCamPreset(p);

	}, int(g_scriptconfig.get<float>("camFollowVehicleCameraMetadata", "HighSpeedShakeSpeed", 40.0f) != FLT_MAX), 1)));

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
	if (settingIndex >= 237)
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
	if (settingIndex >= 237)
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

	for (auto it = g_customPrefs.begin(); it != g_customPrefs.end(); it++)
	{
		SetPauseMenuPreference_Stub(it->first, it->second.m_resetvalue, 3u);
	}
}

void doCameraPatches()
{
	LOG("doCameraPatches(): Begin patches...");

	patchMetadataCollection();

	if (!g_scriptconfig.get<bool>("InVehicleCamera", "SwapCameraOnWaterEnter", true))
	{
		g_cinematicCameraEnterWaterPatch1.install();
	}

	if (!g_scriptconfig.get<bool>("InVehicleCamera", "SwapCameraOnVehicleDestroyed", true))
	{
		g_cinematicCameraEnterWaterPatch2.install();
	}

	LOG("doCameraPatches(): Patches completed!");
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

void checkCameraFrame()
{
	Entity camEntity;
	bool inVehicle;
	unsigned int modelHash;

	auto playerPed = PLAYER::PLAYER_PED_ID();

	if (PED::IS_PED_IN_ANY_VEHICLE(playerPed, true))
	{
		camEntity = AI::GET_IS_TASK_ACTIVE(playerPed, 2) ?
			PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(playerPed) :
			PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
		modelHash = ENTITY::GET_ENTITY_MODEL(camEntity);
		inVehicle = true;
	}

	else
	{
		//	auto directorObjHash = getHashKey("camGameplayDirector");
		modelHash = ENTITY::GET_ENTITY_MODEL(playerPed);	
		camEntity = playerPed;
		inVehicle = false;
	}

	if (!modelHash) return;

	//if (!camDirector) return;

	if (modelHash != g_modelId)
	{
		g_modelId = modelHash;

		LOG("checkCameraFrame(): Looking up preset data using key %d...", modelHash);

		auto itPreset = g_camPresets.find(modelHash);

		if (itPreset != g_camPresets.end())
		{
			LOG("checkCameraFrame(): Found camera presets. applying...");

			printToScreen("Updating camera settings...");

			for (auto it = itPreset->second.begin(); it != itPreset->second.end(); it++)
			{
				auto addresses = g_addresses.get(g_metadataHashMap[it->metadataHash]);

				if (!addresses)
				{
					LOG("checkCameraFrame(): Couldn't find offsets for metadata (0x%lX)...", it->metadataHash);
					continue;
				}

				unsigned int cameraHash;

				if (inVehicle)
				{
					auto modelInfo = *reinterpret_cast<uintptr_t*>(getScriptHandleBaseAddress(camEntity) + 0x20);

					if (!modelInfo)
					{
						LOG("checkCameraFrame(): Couldn't find model info for vehicle.");
						return;
					}

					if (it->metadataHash == eCamCinematicMountedCameraMetadata)
					{
						cameraHash = *reinterpret_cast<DWORD*>(modelInfo + 0x4AC);
					}

					else if (it->metadataHash == eCamFollowVehicleCameraMetadata)
					{
						cameraHash = *reinterpret_cast<DWORD*>(modelInfo + 0x4A0);
					}

					else
					{
						cameraHash = 0;
						LOG("checkCameraFrame(): Invalid metadata type supplied for vehicle (0x%lX).", it->metadataHash);
						return;
					}
				}

				else
				{
					if (it->metadataHash == eCamFollowPedCameraMetadata)
					{
						cameraHash = 0xFBE36564;
					}

					else if (it->metadataHash == eCamFirstPersonShooterCameraMetadata)
					{
						cameraHash = 0xA70102CA;
					}

					else
					{
						cameraHash = 0;
						LOG("checkCameraFrame(): Invalid metadata type supplied for ped (0x%lX).", it->metadataHash);
						return;
					}
				}

				auto camObjMetadata = getMetadataForHash(cameraHash);

				if (!camObjMetadata)
				{
					LOG("checkCameraFrame(): No metadata for hash 0x%lX.", camObjMetadata);
					continue;
				}

				else 
					LOG("checkCameraFrame(): Got pointer to metadata (%p).", camObjMetadata);

				auto psoStruct = camObjMetadata->getPsoStruct();

				if (!psoStruct) continue;

				auto metadataTypeHash = *reinterpret_cast<DWORD*>(psoStruct + 8);

				if (metadataTypeHash != it->metadataHash)
				{
					LOG("checkCameraFrame(): Found metadata type didn't match ours. Ours was 0x%lX but we found 0x%lX.", it->metadataHash, metadataTypeHash);
					continue;
				}

				auto address = reinterpret_cast<uintptr_t>(camObjMetadata) + addresses->map[it->name];

				switch (it->type)
				{
				case CPT_BOOLEAN:
					WriteBool(address, it->value.enabled);
				case CPT_INTEGER:
					WriteInt(address, it->value.integer);
				case CPT_UINTEGER:
					WriteUInt(address, it->value.unsignedInt);
				case CPT_FLOAT:
					WriteFloat(address, it->value.fvalue);
				case CPT_DOUBLE:
					WriteDouble(address, it->value.dvalue);
				default:
					break;
				}
			}

			LOG("checkCameraFrame(): Finished writing data...");
		}
	}
}

void mainLoop()
{
	while (true)
	{
		WAIT(0);

		checkCameraFrame();

		if (bInitialized || *g_gameState != Playing) continue;

		doCameraPatches();

		validateAppVersion();

		if (g_scriptconfig.get<bool>("General", "UseCameraPresets", true))
		{
			bUseCameraIndependentSettings = true;
		}

		printToScreen("Loaded");

		bInitialized = true;
	}
}

void setupHooks()
{
	auto gameVersion = getGameVersion();

	// invalid game version
	if (gameVersion == VER_UNK) return;

	makeConfigFile();

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
	memset((void*)pattern.get(0x12), 0x90, 6);

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

	getCamDirectorObjectByHash = (GetCamDirectorObjectByHash_t)result;
}

void scriptMain()
{
	readXmlDataFileEntries("CameraPresets.xml");

	setupHooks();

	addPauseMenuItems();

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
	removePauseMenuItems();

	removePatches();

	removeHooks();
}
