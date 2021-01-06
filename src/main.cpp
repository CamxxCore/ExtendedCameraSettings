#include "stdafx.h"

#pragma warning(disable : 4244 4305)

#define DLL_EXPORT extern "C" __declspec( dllexport )

bytepatch_t g_enterWaterSwapPatch,
g_vehicleDestroyedSwapPatch,
g_autoRotatePatch,
g_aimCamFieldOfViewPatch;

std::map<unsigned int, std::string> g_textEntries;

static std::mutex g_textMutex;

Game g_game;

CConfig g_scriptconfig;

const int kMenuItemsCount = 27;
const int kSettingsStartIndex = 175;

bool bDidLoad = false;
bool bPostLoadFinished = false;

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

camBaseDirector* g_camGameplayDirector = nullptr;

DWORD vkReloadPresets = VK_B, vkSaveLayout = VK_F11;

std::map<int, std::string> g_dataFileTypeMap = {
    { CPT_BOOLEAN, "bool" },
    { CPT_INTEGER, "int" },
    { CPT_UINTEGER, "uint" },
    { CPT_FLOAT, "float" },
    { CPT_DOUBLE, "double" }
};

std::map<eMetadataHash, std::string> g_metadataHashMap = {
    { eCamFirstPersonShooterCameraMetadata, "firstPersonPedCam" },
    { eCamFollowVehicleCameraMetadata, "thirdPersonVehicleCam" },
    { eCamFollowPedCameraMetadata, "thirdPersonPedCam" },
    { eCamCinematicMountedCameraMetadata, "firstPersonVehicleCam" },
    { eCamThirdPersonPedAimCameraMetadata, "thirdPersonAimCam" },
};

void printToScreen(const char* format, ...) {
    char inBuf[256];

    va_list va;
    va_start(va, format);
    vsprintf_s(inBuf, format, va);
    va_end(va);

    char szText[256];

    auto prefix = Utility::GetModuleName(Utility::GetActiveModule());

    sprintf_s(szText, "~y~%s~w~\n%s", prefix.c_str(), inBuf);

    notifyAboveMap(szText);
}

void addOffsetsForGameVersion(int version) {
#pragma region firstPersonCamera

    auto offsets = g_addresses.getOrCreate("firstPersonPedCam");

    offsets->insert("fov", 36); //F3 44 0F 5C 48 ? F3 41 0F 59 F8
    offsets->insert("aimingFov", 560); //F3 0F 10 80 ? ? ? ? 76 25 
    offsets->insert("minPitch", 84); //F3 0F 10 40 ? F3 0F 59 05 ? ? ? ? F3 0F 11 87 ? ? ? ?
    offsets->insert("maxPitch", 88); //+4
    offsets->insert("alwaysUseReticle", 114); //44 38 79 72
    offsets->insert("viewOffsetX", 64); //F3 0F 10 4A ? F3 0F 10 42 ? 44 0F 29 ? ? ? ? ?
    offsets->insert("viewOffsetY", offsets->map["viewOffsetX"].add(4));
    offsets->insert("viewOffsetZ", offsets->map["viewOffsetX"].add(8));
    offsets->insert("altMinYaw", version < VER_1_0_505_2_STEAM ? 696 : version < VER_1_0_877_1_STEAM ? 712 : 760); //48 8B 81 ? ? ? ? EB E6
    offsets->insert("altMaxYaw", offsets->map["altMinYaw"].add(4)); //+4
    offsets->insert("altMinPitch", offsets->map["altMinYaw"].add(8)); //+8
    offsets->insert("altMaxPitch", offsets->map["altMinYaw"].add(12)); //+C

#pragma endregion

#pragma region cinematicMountedCamera

    offsets = g_addresses.getOrCreate("firstPersonVehicleCam");

    offsets->insert("fov", version < VER_1_0_877_1_STEAM ? 80 : 84); //F3 0F 10 48 ? F3 0F 11 89 ? ? ? ?
    offsets->insert("minPitch", version < VER_1_0_505_2_STEAM ? 808 : version < VER_1_0_877_1_STEAM ? 824 : version < VER_1_0_944_2_STEAM ? 872 : 888); //F3 0F 10 ? ? ? ? ? 0F 2F D0 72 10 F3 0F 10 ? ? 03 00 00
    offsets->insert("maxPitch", offsets->map["minPitch"].add(4));
    offsets->insert("minPitchExt", version < VER_1_0_505_2_STEAM ? 776 : version < VER_1_0_877_1_STEAM ? 792 : version < VER_1_0_944_2_STEAM ? 840 : 856); //F3 0F 59 87 ? ? ? ? F3 41 0F 59 C4
    offsets->insert("maxPitchExt", version < VER_1_0_505_2_STEAM ? 780 : version < VER_1_0_877_1_STEAM ? 796 : version < VER_1_0_944_2_STEAM ? 844 : 860);
    offsets->insert("minSpeedForAutoCorrect", version < VER_1_0_505_2_STEAM ? 680 : version < VER_1_0_877_1_STEAM ? 696 : version < VER_1_0_944_2_STEAM ? 744 : 760); //0F 2F B0 ? ? ? ? 0F 82 ? 02 00 00
    offsets->insert("viewOffsetX", version < VER_1_0_877_1_STEAM ? 80 : 96);  //F3 44 0F 10 ? ? F3 44 0F 10 ? ? F3 44 0F 10 ? ? F3 0F 11 45 ? ? 84 ?
    offsets->insert("viewOffsetY", offsets->map["viewOffsetX"].add(4));
    offsets->insert("viewOffsetZ", offsets->map["viewOffsetX"].add(8));

#pragma endregion

#pragma region followVehicleCamera

    offsets = g_addresses.getOrCreate("thirdPersonVehicleCam");

    offsets->insert("fov", 48); //F3 0F 59 48 ? 0F 2F C8 72 2C
    offsets->insert("highSpeedShakeSpeed", version < VER_1_0_505_2_STEAM ? 1176 : version < VER_1_0_944_2_STEAM ? 1192 : version < VER_1_0_1290_1_STEAM ? 1208 : 1272); //48 81 C7 ? ? ? ? 8B 6F 08 shakesettings(0x4E8)+0x10
    offsets->insert("enableAutoCenter", version < VER_1_0_505_2_STEAM ? 877 : version < VER_1_0_944_2_STEAM ? 893 : version < VER_1_0_1290_1_STEAM ? 909 : 973);// //80 ? ? ? ? ? ? 75 14 48 8B 83 ? ? ? ?
    offsets->insert("autoCenterLerpScale", version < VER_1_0_505_2_STEAM ? 892 : version < VER_1_0_944_2_STEAM ? 908 : version < VER_1_0_1290_1_STEAM ? 924 : 988); //F3 0F 10 88 ? ? ? ? 73 06
    offsets->insert("followDistance", version < VER_1_0_791_2_STEAM ? 312 : version < VER_1_0_944_2_STEAM ? 320 : version < VER_1_0_1290_1_STEAM ? 328 : 360); //F3 0F 10 80 ? ? ? ? C3 4C 8B 81 ? ? ? ? 49 81 C0 ? ? ? ?
    offsets->insert("pivotScale", version < VER_1_0_791_2_STEAM ? 164 : version < VER_1_0_944_2_STEAM ? 172 : version < VER_1_0_1290_1_STEAM ? 180 : 188); //F3 0F 10 88 ? ? ? ? 0F 28 C1
    offsets->insert("pivotOffsetX", version < VER_1_0_791_2_STEAM ? 232 : version < VER_1_0_944_2_STEAM ? 240 : version < VER_1_0_1011_1_STEAM ? 256 : version < VER_1_0_1290_1_STEAM ? 248 : 280); //F3 0F 10 B0 ? ? ? ? F3 0F 10 B8 ? ? ? ? 48 8B 05 ? ? ? ?
    offsets->insert("speedZoomStartSpeed", version < VER_1_0_505_2_STEAM ? 1136 : version < VER_1_0_944_2_STEAM ? 1152 : version < VER_1_0_1290_1_STEAM ? 1168 : 1232); //48 8B BB ? ? ? ? F3 0F 10 3D ? ? ? ? 48 81 C7 ? ? ? ?
    offsets->insert("speedZoomEndSpeed", offsets->map["speedZoomStartSpeed"].add(4));
#pragma endregion

#pragma region followPedCamera

    offsets = g_addresses.getOrCreate("thirdPersonPedCam");

    offsets->insert("fov", 48);
    offsets->insert("sprintShakeSpeed", version < VER_1_0_505_2_STEAM ? 2068 : version < VER_1_0_944_2_STEAM ? 2092 : version < VER_1_0_1290_1_STEAM ? 2108 : 2140);
    offsets->insert("enableAutoCenter", version < VER_1_0_505_2_STEAM ? 877 : version < VER_1_0_944_2_STEAM ? 893 : version < VER_1_0_1290_1_STEAM ? 909 : 973);// //80 ? ? ? ? ? ? 75 14 48 8B 83 ? ? ? ?
    offsets->insert("minPitch", version < VER_1_0_505_2_STEAM ? 580 : version < VER_1_0_944_2_STEAM ? 596 : version < VER_1_0_1290_1_STEAM ? 612 : 644);
    offsets->insert("maxPitch", version < VER_1_0_505_2_STEAM ? 584 : version < VER_1_0_944_2_STEAM ? 600 : version < VER_1_0_1290_1_STEAM ? 616 : 648);
    offsets->insert("pivotOffsetX", version < VER_1_0_791_2_STEAM ? 232 : version < VER_1_0_944_2_STEAM ? 240 : /*version < VER_1_0_1290_1_STEAM ? */256 /*: 248*/);//
    offsets->insert("followDistance", version < VER_1_0_791_2_STEAM ? 312 : version < VER_1_0_944_2_STEAM ? 320 : version < VER_1_0_1290_1_STEAM ? 328 : 360);

#pragma endregion

#pragma region pedAimCamera

    offsets = g_addresses.getOrCreate("thirdPersonAimCam");

    offsets->insert("fov", 48);
    offsets->insert("useWeaponFovSetting", version < VER_1_0_505_2_STEAM ? 727 : version < VER_1_0_944_2_STEAM ? 743 : version < VER_1_0_1290_1_STEAM ? 759 : 827); //80 B8 ? ? ? ? ? 74 15 48 8B 81 ? ? ? ?
    offsets->insert("pivotOffsetX", version < VER_1_0_791_2_STEAM ? 232 : version < VER_1_0_944_2_STEAM ? 240 : version < VER_1_0_1011_1_STEAM ? 256 : 248);
    offsets->insert("followDistance", version < VER_1_0_791_2_STEAM ? 312 : version < VER_1_0_944_2_STEAM ? 320 : version < VER_1_0_1290_1_STEAM ? 328 : 360);

#pragma endregion

#pragma region CVehicleModelInfo

    offsets = g_addresses.getOrCreate("CVehicleModelInfo");

    offsets->insert("thirdPersonCameraHash", version < VER_1_0_505_2_STEAM ? 1120 : version < VER_1_0_791_2_STEAM ? 1168 : version < VER_1_0_1290_1_STEAM ? 1184 : 1232);//48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 41 56 41 57 48 8B EC 48 83 EC 40 49 89 10 48 8B 81 ? ? ? ? 45 33 FF
    offsets->insert("firstPersonCameraHash", offsets->map["thirdPersonCameraHash"].add(0xC));
#pragma endregion
}

void readPresetsFromFile(const std::string& filename) {
    auto filePath = Utility::GetWorkingDirectory() + filename;

    g_camPresets.clear();

    tinyxml2::XMLDocument doc;

    const auto result = doc.LoadFile(filePath.c_str());

    if (result != XML_SUCCESS) {
        switch (result) {
        case XML_ERROR_FILE_NOT_FOUND: {
            LOG("Creating new document %s", filename.c_str());
            auto pRoot = doc.NewElement("cameraPresets");
            doc.InsertFirstChild(pRoot);
            doc.SaveFile(filePath.c_str());
            break;
        }

        case XML_ERROR_FILE_READ_ERROR:
            LOG("readPresetsFromFile(): Encountered a read error loading the file %s (%d)", filename.c_str(), result);
            return;

        case XML_ERROR_EMPTY_DOCUMENT:
            LOG("readPresetsFromFile(): Found an xml file but it contains no text!");
            return;

        case XML_ERROR_FILE_COULD_NOT_BE_OPENED:
        default:
            LOG("readPresetsFromFile(): Encountered an error loading the file %s (%d)", filename.c_str(), result);
            return;
        }
    }

    auto rootElement = doc.FirstChildElement("cameraPresets");

    if (!rootElement) {
        LOG("readPresetsFromFile(): No root element.");
        return;
    }

    XMLHelper::ForEach(rootElement, "camPreset", [](XMLElement* e) {

        auto modelName = e->FirstChildElement("modelName")->GetText();

        std::vector<CamMetadataPreset> presets;

        for (auto it = g_metadataHashMap.begin(); it != g_metadataHashMap.end(); ++it) {
            auto element = XMLHelper::FirstElement(e, it->second);

            if (!element) continue;

            LOG("Adding %s presets for model %s", it->second.c_str(), modelName);

            for (auto p = element->FirstChildElement("Preset"); p != nullptr; p = p->NextSiblingElement("Preset")) {
                CamMetadataPreset preset;

                preset.name = std::string(p->Attribute("name"));

                preset.metadataHash = static_cast<eMetadataHash>(it->first);

                auto type = p->Attribute("type");

                auto value = p->GetText();

                std::stringstream sstream;

                if (!strcmp(type, "bool")) {
                    preset.type = CPT_BOOLEAN;

                    sstream << std::boolalpha << value;
                    sstream >> preset.value.enabled;
                }

                else if (!strcmp(type, "int")) {
                    preset.type = CPT_INTEGER;

                    sstream << value;
                    sstream >> preset.value.integer;
                }

                else if (!strcmp(type, "uint")) {
                    preset.type = CPT_UINTEGER;

                    sstream << value;
                    sstream >> preset.value.unsignedInt;
                }

                else if (!strcmp(type, "float")) {
                    preset.type = CPT_FLOAT;

                    sstream << value;
                    sstream >> preset.value.fvalue;
                }

                else if (!strcmp(type, "double")) {
                    preset.type = CPT_DOUBLE;

                    sstream << value;
                    sstream >> preset.value.dvalue;
                }

                else {
                    LOG("Invalid preset type provided for item %s (%s)", preset.name.c_str(), type);
                    continue;
                }

                presets.push_back(preset);

                LOG("<Preset name=\"%s\" type=\"%s\">%s</Preset>", preset.name.c_str(), type, value);
            }
        }

        g_camPresets.insert(make_pair(Utility::StringToHash(modelName), presets));
        });
}

void writePresetsToFile(const std::string& filename, unsigned int modelHash, std::vector<CamMetadataPreset> presets) {

    auto filePath = Utility::GetWorkingDirectory() + filename;

    if (!Utility::FileExists(filePath)) {
        LOG("writePresetsToFile(): File not found (%s)", filePath.c_str());
        return;
    }

    tinyxml2::XMLDocument doc;

    auto result = doc.LoadFile(filePath.c_str());

    if (result != XML_SUCCESS) {
        LOG("writePresetsToFile(): Encountered an error loading the file %s (%d)", filename.c_str(), result);
        return;
    }

    auto rootElement = doc.FirstChildElement("cameraPresets");

    if (!rootElement) {
        LOG("writePresetsToFile(): No root element.");
        return;
    }

    auto elem = XMLHelper::FindIf(rootElement, "camPreset", [modelHash](XMLElement* e) -> bool {
        auto str = e->FirstChildElement("modelName")->GetText();
        return modelHash == Utility::StringToHash(str);
        });

    if (!elem) {
        LOG("writePresetsToFile(): No entry found for hash. Create a new one...");

        elem = doc.NewElement("camPreset");

        auto subNode = doc.NewElement("modelName");

        std::string modelName;

        STREAMING::IS_MODEL_A_VEHICLE(modelHash) ?
            getVehicleModelName(modelHash, modelName) : getPedModelName(modelHash, modelName);

        subNode->SetText(modelName.c_str());

        elem->LinkEndChild(subNode);

        rootElement->LinkEndChild(elem);
    }

    LOG("writePresetsToFile(): Setting presets...");

    for (auto it = presets.begin(); it != presets.end(); ++it) {
        auto nameStr = g_metadataHashMap[it->metadataHash];

        auto childElem = XMLHelper::FirstElement(elem, nameStr);

        if (!childElem) {
            LOG("writePresetsToFile(): Element contains no %s node. Creating a new child node...", nameStr.c_str());

            childElem = doc.NewElement(nameStr.c_str());

            elem->LinkEndChild(childElem);
        }

        auto node = XMLHelper::FindByAttribute(childElem, "Preset", "name", it->name);

        if (node) {
            LOG("writePresetsToFile(): Preset node exists. setting value...");

            node->SetText(it->toString().c_str());

            continue;
        }

        LOG("writePresetsToFile(): No preset node found. Creating a new one...");

        node = doc.NewElement("Preset");

        LOG("name: %s", it->name.c_str());

        node->SetAttribute("name", it->name.c_str());

        node->SetAttribute("type", g_dataFileTypeMap[it->type].c_str());

        node->SetText(it->toString().c_str());

        childElem->LinkEndChild(node);
    }

    doc.SaveFile(filePath.c_str());
}

void writePresetToFile(const std::string& filename, unsigned int modelHash, CamMetadataPreset preset) {
    writePresetsToFile(filename, modelHash, std::vector<CamMetadataPreset> { preset });
}

static CallHook<const char* (*)(void*, unsigned int)>* g_getGxtEntry;

const char* getGxtEntry_Hook(void* unk, unsigned int hashName) {
    std::unique_lock<std::mutex> lock(g_textMutex);

    const auto it = g_textEntries.find(hashName);

    if (it != g_textEntries.end()) {
        return it->second.c_str();
    }

    return g_getGxtEntry->fn(unk, hashName);
}

unsigned int addGxtEntry(std::string key, const std::string& text) {
    const auto hashKey = Utility::GetHashKey(key);

    std::unique_lock<std::mutex> lock(g_textMutex);

    g_textEntries[hashKey] = text;

    return hashKey;
}

inline UIMenu* lookupMenuForIndex(int menuIndex) {
    auto menuPool = g_game.GetActiveMenuPool();

    for (auto it = menuPool->begin(); it != menuPool->end(); it++) {
        if (!it || it->menuId != menuIndex)
            continue;
        return it;
    }

    return nullptr;
}

camBaseObjectMetadata* getCamMetadataForHash(unsigned int hashName) {
    auto metadataPool = g_game.GetCamMetadataPool();

    for (auto it = metadataPool->begin(); it != metadataPool->end(); it++) {
        if (!it) continue;

        for (auto ref = *it; ref; ref = ref->pNext) {
            if (!ref->pData) continue;

            if (ref->pData->hashKey == hashName) {

                return ref->pData;
            }
        }
    }

    return nullptr;
}

void setupMenuItem(UIMenuItem* item, const std::string& text, int menuIndex, int type, int actionType, int settingIdx, int stateFlags) {
    item->textHash = addGxtEntry(text, text);
    item->menuIndex = menuIndex;
    item->type = type;
    item->actionType = actionType;
    item->settingId = static_cast<unsigned char>(settingIdx);
    item->stateFlags = stateFlags;
}

void saveCamPresets() {
    const auto keyValue = bUseGlobalPresets ? 0 : g_modelId;

    const auto presets = g_camPresets.find(keyValue);

    if (presets == g_camPresets.end()) return;

    writePresetsToFile("CameraPresets.xml", keyValue, presets->second);

    if (bVehicleCamActive && !bUseGlobalPresets) {
        const auto szDisplayName = VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(keyValue);

        printToScreen("Settings saved for %s", UI::_GET_LABEL_TEXT(szDisplayName));
    }

    else printToScreen("Camera settings saved");
}

bool checkCamPresetKey(eMetadataHash type, unsigned int* newKeyValue) {
    if (bUseGlobalPresets) {
        *newKeyValue = 0;
    }

    else {
        if (bVehicleCamActive && (type == eCamFollowPedCameraMetadata ||
            type == eCamFirstPersonShooterCameraMetadata ||
            type == eCamThirdPersonPedAimCameraMetadata)) {
            *newKeyValue = ENTITY::GET_ENTITY_MODEL(PLAYER::PLAYER_PED_ID());
            return false;
        }

        if (!bVehicleCamActive && (type == eCamFollowVehicleCameraMetadata ||
            type == eCamCinematicMountedCameraMetadata)) {
            // set global vehicle presets if we aren't sitting in a vehicle.
            *newKeyValue = 0;
            return false;
        }

        *newKeyValue = g_modelId;
    }

    return true;
}

bool getCamPreset(eMetadataHash eType, std::string& name, CamMetadataPreset* preset) {
    unsigned int keyValue;

    checkCamPresetKey(eType, &keyValue);

    auto items = &g_camPresets[keyValue];

    const auto it = std::find_if(items->begin(), items->end(), [&eType, &name](const CamMetadataPreset& p) {
        return eType == p.metadataHash && name == p.name;
        });

    if (it == items->end())
        return false;

    LOG("getCamPreset(): Found existing preset %s", it->name.c_str());

    if (preset) {
        *preset = *it;
    }

    return true;
}

void setCamPresetForKey(unsigned int hashKey, CamMetadataPreset& preset, bool writeToFile) {

    auto items = &g_camPresets[hashKey];

    auto it = std::find_if(items->begin(), items->end(),
        [&preset](const CamMetadataPreset& p) {
            return preset.metadataHash == p.metadataHash && preset.name == p.name;
        });

    if (it != items->end()) {
        it->value = preset.value;
    }

    else
        items->push_back(preset);

    if (!writeToFile) return;

    writePresetToFile("CameraPresets.xml", hashKey, preset);
}

void setCamPreset(CamMetadataPreset preset) {
    unsigned int keyValue;

    if (checkCamPresetKey(preset.metadataHash, &keyValue))
        bShouldUpdatePresets = true;

    setCamPresetForKey(keyValue, preset, bAutoSaveLayouts);
}

float getPresetValueFloat(eMetadataHash type, std::string name, float defaultValue) {
    CamMetadataPreset p;
    if (!getCamPreset(type, name, &p))
        return defaultValue;
    return p.value.fvalue;
}

bool getPresetValueBool(eMetadataHash type, std::string name, bool defaultValue) {
    CamMetadataPreset p;
    if (!getCamPreset(type, name, &p))
        return defaultValue;
    return p.value.enabled;
}

int getLanguageTextId() {
    const auto uiLanguage = UNK::_GET_UI_LANGUAGE_ID();

    switch (uiLanguage) {
    case 0:
        return 0;
    case 4:
        return 1;
    case 7:
        return 2;
    case 9:
    case 12:
        return 3;
    default:
        return 0;
    }
}

void addPauseMenuItems() {
    LOG("addPauseMenuItems(): Begin add menu items...");

    auto const cameraSettingsLutIdx = 136;

    LOG("addPauseMenuItems(): Performing lookup for menu index (%d)...", cameraSettingsLutIdx);

    auto pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

    auto newCount = pMenu->itemCount + kMenuItemsCount;

    auto newSize = newCount * sizeof(UIMenuItem);

    auto newItemArray = new UIMenuItem[newCount];

    memcpy_s(newItemArray, newSize, pMenu->items, pMenu->itemCount * sizeof(UIMenuItem));

    // overwrite the reset button since we will move it to the bottom later..
    auto itemIdx = pMenu->itemCount - 1;

    // hijack the last 50 settings indices with the hope that
    // rockstar won't add more than 15 new settings in the future.. (max is 255 or CHAR_MAX)
    auto targetSettingIdx = kSettingsStartIndex;

    // add menu settings (kMenuItemsCount needs to be updated to reflect changes made here)

    const auto languageId = getLanguageTextId();

#pragma region FP Use Reticle

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_USE_RETICLE), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFirstPersonShooterCameraMetadata;
        p.name = std::string("alwaysUseReticle");
        p.type = CPT_BOOLEAN;
        p.value.enabled = value != 0;

        setCamPreset(p);

        }, []() -> int { return int(getPresetValueBool(eCamFirstPersonShooterCameraMetadata, std::string("alwaysUseReticle"), false)); }, 0)));

#pragma endregion

#pragma region FP Max Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFirstPersonShooterCameraMetadata;
        p.name = std::string("minPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(-getPresetValueFloat(eCamFirstPersonShooterCameraMetadata, std::string("minPitch"), -60.0f), 40.0f, 80.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FP Min Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFirstPersonShooterCameraMetadata;
        p.name = std::string("maxPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 40.0f, 80.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFirstPersonShooterCameraMetadata, std::string("maxPitch"), 60.0f), 40.0f, 80.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FP Aiming FOV

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_AIMING_FOV), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        if (!g_aimCamFieldOfViewPatch.active)
            // override weapon config
            g_aimCamFieldOfViewPatch.install();
        else if (value == 5)
            // default value, use weapon specific fov.
            g_aimCamFieldOfViewPatch.remove();

        CamMetadataPreset p;
        p.metadataHash = eCamFirstPersonShooterCameraMetadata;
        p.name = std::string("aimingFov");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 12.0f, 72.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFirstPersonShooterCameraMetadata, std::string("aimingFov"), 42.0f), 12.0f, 72.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPP FOV

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_FOV), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("fov");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 20.0f, 80.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("fov"), 50.0f), 20.0f, 80.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPP Auto Center

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_AUTO_CENTER), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("enableAutoCenter");
        p.type = CPT_BOOLEAN;
        p.value.enabled = value != 0;

        setCamPreset(p);

        }, []() -> int { return getPresetValueBool(eCamFollowPedCameraMetadata, std::string("enableAutoCenter"), true); }, 1)));

#pragma endregion

#pragma region TPP Horizontal Origin

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("pivotOffsetX");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.1f, 0.9f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("pivotOffsetX"), 0.4f), -0.1f, 0.9f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPP Follow Distance

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("followDistance");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0.0f, 2.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("followDistance"), 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPP Running Shake

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_SHAKE), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("sprintShakeSpeed");
        p.type = CPT_FLOAT;
        p.value.fvalue = value != 0 ? 0.5f : FLT_MAX;

        setCamPreset(p);

        }, []() -> int { return int(getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("sprintShakeSpeed"), 0.5f) != FLT_MAX); }, 1)));

#pragma endregion

#pragma region TPP Max Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("minPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, 50.0f, 90.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(-getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("minPitch"), -70.0f), 50.0f, 90.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPP Min Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowPedCameraMetadata;
        p.name = std::string("maxPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 1.0f, 89.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowPedCameraMetadata, std::string("maxPitch"), 45.0f), 0.0f, 90.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPA FOV

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_AIMING_FOV), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
        p.name = std::string("fov");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 15.0f, 75.0f);

        setCamPreset(p);

        p.name = std::string("useWeaponFovSetting");
        p.type = CPT_BOOLEAN;
        p.value.enabled = value == 5 ? true : false;

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, std::string("fov"), 45.0f), 15.0f, 75.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPA Horizontal Origin

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_AIMING_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
        p.name = std::string("pivotOffsetX");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.1f, 0.9f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, std::string("pivotOffsetX"), 0.4f), -0.1f, 0.9f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPA Follow Distance

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_AIMING_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamThirdPersonPedAimCameraMetadata;
        p.name = std::string("followDistance");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0.0f, 2.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamThirdPersonPedAimCameraMetadata, std::string("followDistance"), 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FPV FOV

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_FOV), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamCinematicMountedCameraMetadata;
        p.name = std::string("fov");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, std::string("fov"), 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FPV Vertical Origin

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_VERT_OFF), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamCinematicMountedCameraMetadata;
        p.name = std::string("viewOffsetZ");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.05f, 0.05f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, std::string("viewOffsetZ"), 0.0f), -0.05f, 0.05f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FPV Switch In Water

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_SWITCH_WATER), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        if (value) {
            g_enterWaterSwapPatch.remove();
        }
        else
            g_enterWaterSwapPatch.install();

        g_scriptconfig.set<bool>("GlobalSettings", "SwapCameraOnWaterEnter", value != 0);

        }, []() -> int { return g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnWaterEnter", true); }, 1)));

#pragma endregion

#pragma region FPV Switch On Desroyed

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_SWITCH_DAMAGE), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        if (value) {
            g_vehicleDestroyedSwapPatch.remove();
        }
        else
            g_vehicleDestroyedSwapPatch.install();

        g_scriptconfig.set<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", value != 0);

        }, []() -> int { return g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", true); }, 1)));

#pragma endregion

#pragma region FPV Min Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_MIN_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamCinematicMountedCameraMetadata;
        p.name = std::string("minPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = -Math::FromToRange(value, 0.0f, 10.0f, -30.0f, 50.0f);

        setCamPreset(p);

        p.name = std::string("minPitchExt");
        setCamPreset(p);
        }, []() -> int { return int(Math::FromToRange(-getPresetValueFloat(eCamCinematicMountedCameraMetadata, std::string("minPitch"), -10.0f), -30.0, 50.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region FPV Max Pitch Limit

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_FP_VEHICLE_MAX_PITCH), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamCinematicMountedCameraMetadata;
        p.name = std::string("maxPitch");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -25.0f, 55.0f);

        setCamPreset(p);

        p.name = std::string("maxPitchExt");
        setCamPreset(p);
        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamCinematicMountedCameraMetadata, std::string("maxPitch"), 15.0f), -25.0, 55.0, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPV Field Of View

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_FOV), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("fov");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 30.0f, 70.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("fov"), 50.0f), 30.0f, 70.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPV Auto Center

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_AUTO_CENTER), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("enableAutoCenter");
        p.type = CPT_BOOLEAN;
        p.value.enabled = value != 0;

        if (value)
            g_autoRotatePatch.remove();
        else
            g_autoRotatePatch.install();

        setCamPreset(p);

        }, []() -> int { return getPresetValueBool(eCamFollowVehicleCameraMetadata, std::string("enableAutoCenter"), true); }, 1)));

#pragma endregion

#pragma region TPV Follow Distance

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_FOLLOW_DIST), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("followDistance");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.925f, 3.075f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("followDistance"), 1.075f), -0.925f, 3.075f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPV Follow Distance

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_SPEED_ZOOM), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("speedZoomStartSpeed");
        p.type = CPT_FLOAT;
        p.value.fvalue = value != 0 ? 20.0f : FLT_MAX;

        setCamPreset(p);

        p.name = std::string("speedZoomEndSpeed");
        p.value.fvalue = value != 0 ? 35.0f : FLT_MAX;
        setCamPreset(p);

        }, []() -> int { return int(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("speedZoomStartSpeed"), 20.0f) != FLT_MAX); }, 1)));

#pragma endregion

#pragma region TPV Follow Height

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_PIVOT_SCALE), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("pivotScale");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, 0, 2.0f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("pivotScale"), 1.0f), 0.0f, 2.0f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPV Horizontal Origin

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_HORZ_OFF), 51, 2, Slider, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx++, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("pivotOffsetX");
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(value, 0.0f, 10.0f, -0.5f, 0.5f);

        setCamPreset(p);

        }, []() -> int { return int(Math::FromToRange(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("pivotOffsetX"), 0.0f), -0.5f, 0.5f, 0.0f, 10.0f)); }, 5)));

#pragma endregion

#pragma region TPV High Speed Shake

    setupMenuItem(&newItemArray[itemIdx++], getConstString(languageId, MO_TP_VEHICLE_SHAKE), 51, 2, Toggle, targetSettingIdx, 0);

    g_customPrefs.insert(std::make_pair(targetSettingIdx, CustomMenuPref([](int settingIndex, int value) {

        CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = std::string("highSpeedShakeSpeed");
        p.type = CPT_FLOAT;
        p.value.fvalue = value != 0 ? 40.0f : FLT_MAX;

        setCamPreset(p);

        }, []() -> int { return int(getPresetValueFloat(eCamFollowVehicleCameraMetadata, std::string("highSpeedShakeSpeed"), 40.0f) != FLT_MAX); }, 1)));

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

CallHook<bool(*)(int, int, int, int, int, int, const char*, bool, bool)>* g_addSliderFn, * g_addToggleFn;

bool UIMenu__AddItem_Hook(int columnId, int slotIndex, int menuState, int settingIndex, int unk, int value, const char* text, bool bPopScaleform, bool bSlotUpdate) {
    if (settingIndex >= kSettingsStartIndex) {
        const auto it = g_customPrefs.find(settingIndex);

        if (it != g_customPrefs.end()) {
            value = it->second.m_updated();
        }
    }

    return g_addToggleFn->fn(columnId, slotIndex, menuState, settingIndex, unk, value, text, bPopScaleform, bSlotUpdate);
}

CallHook<void(*)(long long, int, unsigned int)>* g_setPauseMenuPreference;

void SetPauseMenuPreference_Hook(long long settingIndex, int value, unsigned int unk) {
    LOG("SetPauseMenuPreference_Hook(%llu, %d, %u)", settingIndex, value, unk);

    if (settingIndex >= kSettingsStartIndex) {
        const auto prefId = int(settingIndex);

        const auto it = g_customPrefs.find(prefId);

        if (it != g_customPrefs.end()) {
            if (it->second.m_callback) {
                it->second.m_callback(prefId, value);
            }
        }
    }

    g_setPauseMenuPreference->fn(settingIndex, value, unk);
}

CallHook<void(*)()>* g_resetCameraSettings;

void ResetCameraProfileSettings_Hook() {
    g_resetCameraSettings->fn();

    LOG("Resetting custom prefs...");

    for (auto it = g_customPrefs.begin(); it != g_customPrefs.end(); ++it) {
        SetPauseMenuPreference_Hook(it->first, it->second.m_resetvalue, 3u);
    }

    saveCamPresets();
}

std::string getResourceConfigData(HMODULE hModule) {
    std::string result;

    HRSRC hRes = FindResource(hModule, MAKEINTRESOURCE(IDR_CFGFILEDATA), RT_RCDATA);

    if (hRes != nullptr) {
        HGLOBAL hData = LoadResource(hModule, hRes);

        if (hData != nullptr) {
            DWORD dataSize = SizeofResource(hModule, hRes);
            char* data = (char*)LockResource(hData);
            result.assign(data, dataSize);
        }
    }

    return result;
}

void makeConfigFile() {
    const auto configPath = g_scriptconfig.getPath();

    if (!Utility::FileExists(configPath)) {
        LOG("makeConfigFile(): Creating new config...");

        printToScreen("Creating config file...");

        const auto hModule = Utility::GetActiveModule();

        auto resText = getResourceConfigData(hModule);

        if (!resText.empty()) {
            std::ofstream ofs(configPath);

            if (ofs.good()) {
                ofs << resText;
                ofs.flush();
                ofs.close();
            }
        }
    }
}

double getRemoteVersionNumber() {
    TCHAR szFilename[MAX_PATH];

    const auto hr = URLDownloadToCacheFileA(nullptr, "http://www.camx.me/gtav/ecs/version.txt", szFilename, MAX_PATH, 0, nullptr);

    if (SUCCEEDED(hr)) {
        std::ifstream file(szFilename, std::ifstream::in);

        double fRemoteVersion;

        if (file.is_open()) {
            file >> fRemoteVersion;

            file.close();

            remove(szFilename);

            return fRemoteVersion;
        }
    }

    return -1.f;
}

void validateAppVersion() {
    if (g_scriptconfig.get<bool>("General", "Notification", false)) {
        const auto fVersionNum = getRemoteVersionNumber();

        if (fVersionNum != -1.0 && fVersionNum > APP_VERSION) {
            printToScreen("A newer version is available (%.2f) Current version: (%.f)", fVersionNum, APP_VERSION);
        }
    }
}

bool getMetadataHashForEntity(Entity entity, eMetadataHash eType, unsigned int* outHash) {

    if (ENTITY::IS_ENTITY_A_VEHICLE(entity)) {

        const auto modelInfo = *reinterpret_cast<uintptr_t*>(getScriptHandleBaseAddress(entity) + 0x20);

        if (!modelInfo)
            return false;

        auto modelOffsets = g_addresses.get("CVehicleModelInfo");

        switch (eType) {
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

    else if (ENTITY::IS_ENTITY_A_PED(entity)) {
        switch (eType) {
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

Hash lastWeaponHash;

void checkCameraFrame() {
    Entity entity;

    const auto playerPed = PLAYER::PLAYER_PED_ID();

    if (g_camGameplayDirector && g_camGameplayDirector->vehicle) {
        entity = AI::GET_IS_TASK_ACTIVE(playerPed, 160) ?
            PED::GET_VEHICLE_PED_IS_TRYING_TO_ENTER(playerPed) :
            PED::GET_VEHICLE_PED_IS_IN(playerPed, false);
        bVehicleCamActive = true;
    }

    else {
        entity = playerPed;
        bVehicleCamActive = false;

        Hash weaponHash;

        if (WEAPON::GET_CURRENT_PED_WEAPON(playerPed, &weaponHash, TRUE)) {
            if (weaponHash != lastWeaponHash) {
                bShouldUpdatePresets = true;

                lastWeaponHash = weaponHash;
            }
        }
    }

    const unsigned int modelHash = ENTITY::GET_ENTITY_MODEL(entity);

    if (!modelHash || modelHash == g_modelId && !bShouldUpdatePresets) return;

    g_modelId = modelHash;

    bShouldUpdatePresets = false;

    const auto keyValue = bUseGlobalPresets ? 0 : g_modelId;

    LOG("checkCameraFrame(): Looking up preset data using key %d...", keyValue);

    auto itPreset = g_camPresets.find(keyValue);

    bool useGlobalPreset = itPreset == g_camPresets.end();

    if (useGlobalPreset) {

        LOG("checkCameraFrame(): Preset lookup failed. Will try to use global preset.");

        itPreset = g_camPresets.find(0);

        if (itPreset == g_camPresets.end()) {
            LOG("checkCameraFrame(): No global preset exists. Exiting...");
            return;
        }
    }

    LOG("checkCameraFrame(): Found camera presets. applying...");

    unsigned int cameraHash;

    for (auto it = itPreset->second.begin(); it != itPreset->second.end(); ++it) {
        auto addresses = g_addresses.get(g_metadataHashMap[it->metadataHash]);

        if (!addresses) {
            LOG("checkCameraFrame(): Couldn't find offsets for metadata (0x%lX)...", it->metadataHash);
            continue;
        }

        if (!getMetadataHashForEntity(entity, it->metadataHash, &cameraHash)) {
            LOG("checkCameraFrame(): Couldn't find camera hash for entity. Base type was %s", g_metadataHashMap[it->metadataHash].c_str());
            continue;
        }

        LOG("checkCameraFrame(): metadataHash 0x%llX", cameraHash);

        auto camObjMetadata = getCamMetadataForHash(cameraHash);

        if (!camObjMetadata) {
            LOG("checkCameraFrame(): No metadata for hash 0x%lX.", cameraHash);
            continue;
        }

        if (IsBadReadPtr(*(LPVOID*)camObjMetadata, 8))
        {
            // LOG("bad read ptr");

            return;
        }

        if (g_scriptconfig.get<bool>("General", "Notification", false) &&
            !useGlobalPreset && STREAMING::IS_MODEL_A_VEHICLE(keyValue))
        {
            std::string modelFriendlyName = "*unknown*";

            modelFriendlyName = UI::_GET_LABEL_TEXT(VEHICLE::GET_DISPLAY_NAME_FROM_VEHICLE_MODEL(keyValue));

            printToScreen("Loading preset for '%s'", modelFriendlyName.c_str());
        }


        // LOG("checkCameraFrame(): Got metadata for hash: %llX", camObjMetadata);

        const auto psoData = camObjMetadata->getPsoStruct();

        if (!psoData) {
            LOG("checkCameraFrame(): Pso structure doesn't exist.");
            continue;
        }

        const auto baseMetadataHash = *reinterpret_cast<DWORD*>(psoData + 8);

        if (baseMetadataHash != it->metadataHash) {
            LOG("checkCameraFrame(): Found metadata type didn't match ours. Ours was 0x%lX but we found 0x%lX.", it->metadataHash, baseMetadataHash);
            continue;
        }

        const auto address = reinterpret_cast<uintptr_t>(camObjMetadata) + addresses->map[it->name];

        switch (it->type) {
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

DLL_EXPORT void ReloadCameraPresets() {
    printToScreen("Reloading camera settings");

    readPresetsFromFile("CameraPresets.xml");

    bShouldUpdatePresets = true;
}

void scriptKeyboardEvent(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow) {
    if (key == vkSaveLayout && GetAsyncKeyState(VK_CONTROL) & 0x8000) {
        saveCamPresets();

        bShouldPlaySound = true;
    }

    else if (key == vkReloadPresets) {
        ReloadCameraPresets();
    }

    else if (key == VK_F9)
    {
        //auto p = getScriptHandleBaseAddress(PLAYER::PLAYER_PED_ID());

        /*CamMetadataPreset p;
        p.metadataHash = eCamFollowVehicleCameraMetadata;
        p.name = "followDistance";
        p.type = CPT_FLOAT;
        p.value.fvalue = Math::FromToRange(5.0f, 0.0f, 10.0f, -0.925f, 3.075f);


        writePresetToFile("CameraPresets.xml", 0, p);
        */
        //printToScreen("wrote preset");

        auto metadata = getCamMetadataForHash(0x623275C3);

        LOG("%p", metadata);
    }
}

void updateFrontendSound() {
    if (bShouldPlaySound) {
        if (GAMEPLAY::GET_GAME_TIMER() - lastSoundPlayedTime > 2000) {
            AUDIO::PLAY_SOUND_FRONTEND(-1, "OTHER_TEXT", "HUD_AWARDS", 1);

            lastSoundPlayedTime = GAMEPLAY::GET_GAME_TIMER();
        }

        bShouldPlaySound = false;
    }
}

void doPostLoad() {
    LOG("Do post load...")

        if (!g_scriptconfig.get<bool>("GlobalSettings", "GamepadFollowCamAutoCenter", true)) {
            g_autoRotatePatch.install();
        }

    if (!g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnWaterEnter", true)) {
        g_enterWaterSwapPatch.install();
    }

    if (!g_scriptconfig.get<bool>("GlobalSettings", "SwapCameraOnVehicleDestroyed", true)) {
        g_vehicleDestroyedSwapPatch.install();
    }

    validateAppVersion();

    if (g_scriptconfig.get<bool>("General", "Notification", false)) {

        printToScreen("Loaded");
    }
}

void mainLoop() {
    while (true) {
        WAIT(0);

        checkCameraFrame();

        updateFrontendSound();

        if (bPostLoadFinished || *g_game.GetGameState() != Playing) continue;

        doPostLoad();

        bPostLoadFinished = true;
    }
}

void setupHooks() {
    const auto gameVersion = getGameVersion();

    LOG("main(): Game version %d", gameVersion);

    // invalid game version
    if (gameVersion == VER_UNK)
        return;

    if (g_game.Initialize(gameVersion)) {

        addOffsetsForGameVersion(gameVersion);

        // jmp patch #1 for stop camera swap on vehicle enter water
        auto result = BytePattern((BYTE*)"\x31\x81\x00\x00\x00\x00\xF3\x0F\x10\x44\x24\x00", "xx????xxxxx?").get().get(gameVersion < VER_1_0_1290_1_STEAM ? 76 : 83);

        if (result) {
            LOG("main(): g_enterWaterPatch1 found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find g_enterWaterPatch1");
            return;
        }

        g_enterWaterSwapPatch = bytepatch_t((PBYTE)result, (gameVersion < VER_1_0_877_1_STEAM&& gameVersion > VER_1_0_505_2_NOSTEAM) ?
            std::vector<BYTE>(6, NOP) : gameVersion < VER_1_0_944_2_STEAM ? std::vector<BYTE> { JMPREL_8 } : gameVersion < VER_1_0_1180_2_STEAM ? std::vector<BYTE>(6, NOP) : std::vector<BYTE>{ JMPREL_8 }); // jz = jmp

// jmp patch #2 for stop camera swap on vehicle enter water
        result = BytePattern((BYTE*)"\x44\x8A\xC5\x48\x8B\x0C\xC8", "xxxxxxx").get().get(48);

        if (result) {
            LOG("main(): g_enterWaterPatch2 found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find g_enterWaterPatch2");
            return;
        }

        g_vehicleDestroyedSwapPatch = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP)); // jz = nop

        result = BytePattern("F3 0F 10 07 0F 28 CF E8 ? ? ? ? 41 0F 28 C8").get().get(-23);

        if (result) {
            LOG("main(): g_autoRotatePatch found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find g_autoRotatePatch");
            return;
        }

        g_autoRotatePatch = bytepatch_t((BYTE*)result, std::vector<BYTE>(6, NOP));

        result = BytePattern((BYTE*)"\x0C\x04\x08\x83\x00\x00\x00\x00", "xxxx????").get().get(61);

        if (result) {
            LOG("main(): g_aimCamFieldOfViewPatch found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find g_aimCamFieldOfViewPatch");
            return;
        }

        g_aimCamFieldOfViewPatch = bytepatch_t((BYTE*)result, std::vector<BYTE> { 0x0F, 0x29, 0xD9, NOP, NOP, NOP, NOP, NOP}); // movaps xmm1, xmm3 (0)

        result = BytePattern((BYTE*)"\x48\x85\xC0\x75\x34\x8B\x0D", "xxxxxxx").get().get(-0x5);

        if (result) {
            LOG("main(): getGxtEntry() found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find getGxtEntry()");
            return;
        }

        g_getGxtEntry = HookManager::SetCall(result, getGxtEntry_Hook);

        BytePattern pattern = BytePattern((BYTE*)"\x83\xFF\x05\x74\x15", "xxxxx");

        result = pattern.get().get(-0x1A);

        if (result) {
            LOG("main(): SetMenuSlot() #1 found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find setMenuSlot() #1");
            return;
        }

        g_addSliderFn = HookManager::SetCall(result, UIMenu__AddItem_Hook); //-0x1A

        result = pattern.get().get(0xA8);

        if (result) {
            LOG("main(): SetMenuSlot() #2 found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find SetMenuSlot() #2");
            return;
        }

        g_addToggleFn = HookManager::SetCall(result, UIMenu__AddItem_Hook); // +0xA8

        pattern = BytePattern((BYTE*)"\x81\xE9\x00\x00\x00\x00\x74\x25\xFF\xC9", "xx????xxxx");

        result = pattern.get().get(-0x2A);

        if (result) {
            LOG("main(): ResetCameraProfileSettings() found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find ResetCameraProfileSettings()");
            return;
        }

        g_resetCameraSettings = HookManager::SetCall(result, ResetCameraProfileSettings_Hook);

        pattern = BytePattern((BYTE*)"\xF2\x0F\x2C\x56\x00", "xxxx?");

        result = pattern.get().get(0x20);

        if (result) {
            LOG("main(): setPauseMenuPreference() found at 0x%llX", result);
        }

        else {
            LOG("[ERROR] main(): Failed to find SetPauseMenuPreference()");
            return;
        }

        g_setPauseMenuPreference = HookManager::SetCall(result, SetPauseMenuPreference_Hook);

        result = pattern.get().get(0x12);

        // always toggle preferences
        memset((void*)result, 0x90, 6);
    }
}

void loadConfigData() {
    char inBuf[MAX_PATH];

    if (g_scriptconfig.getText(inBuf, "Keybinds", "SaveSettings")) {
        vkSaveLayout = keyFromString(inBuf, 0x42);
    }

    if (g_scriptconfig.getText(inBuf, "Keybinds", "ReloadSettings")) {
        vkReloadPresets = keyFromString(inBuf, VK_F11);
    }

    if (!g_scriptconfig.get<bool>("General", "UseCustomPresets", true)) {
        bUseGlobalPresets = true;
    }

    if (g_scriptconfig.get<bool>("General", "AutoSaveLayouts", true)) {
        bAutoSaveLayouts = true;
    }
}

void scriptLoad() {
    if (!bDidLoad) {

        readPresetsFromFile("CameraPresets.xml");

        makeConfigFile();

        LOG("load config");

        loadConfigData();

        LOG("setup hooks");

        setupHooks();

        LOG("check camera frame");

        // pump camera frame for addPauseMenuItems();
        checkCameraFrame();

        addPauseMenuItems();

        bDidLoad = true;
    }

    LOG("Lookup cam director by key...");

    unsigned int camGameplayDirectorHash = 0x3E9ED27Fu;
    // always need to grab this since it will be different after every game reload.
    g_camGameplayDirector = GetCamDirectorFromPool(&camGameplayDirectorHash);

    LOG("Begin loop..");

    // begin infinite loop...
    mainLoop();
}

void removePauseMenuItems() {
    auto const cameraSettingsLutIdx = 136;

    UIMenu* pMenu = lookupMenuForIndex(cameraSettingsLutIdx);

    UIMenuItem* pOriginalItems = pMenu->items;

    const auto newCount = pMenu->itemCount - kMenuItemsCount;

    const auto newSize = newCount * sizeof(UIMenuItem);

    UIMenuItem* newItemArray = new UIMenuItem[newCount];

    memcpy_s(newItemArray, newSize, pMenu->items, newSize);

    // restore reset button
    newItemArray[newCount - 1] = pOriginalItems[pMenu->itemCount - 1];

    pMenu->items = newItemArray;
    pMenu->itemCount = newCount;
    pMenu->maxItems = newCount;
    pMenu->scrollFlags |= 48;

    delete[] pOriginalItems;
}

void removePatches() {
    if (g_enterWaterSwapPatch.active) {
        g_enterWaterSwapPatch.remove();
    }

    if (g_vehicleDestroyedSwapPatch.active) {
        g_vehicleDestroyedSwapPatch.remove();
    }

    if (g_autoRotatePatch.active) {
        g_autoRotatePatch.remove();
    }

    if (g_aimCamFieldOfViewPatch.active) {
        g_aimCamFieldOfViewPatch.remove();
    }
}

template <typename T>
void removeHook(Hook<T>* hook) {
    if (hook) {
        delete hook;
        hook = nullptr;
    }
}

void removeHooks() {
    removeHook(g_getGxtEntry);

    removeHook(g_setPauseMenuPreference);

    removeHook(g_addSliderFn);

    removeHook(g_addToggleFn);

    removeHook(g_resetCameraSettings);
}

void scriptUnload() {
    LOG("Unloading menu items...");

    removePauseMenuItems();

    LOG("Unloading patches...");

    removePatches();

    LOG("Remove hooks...");

    removeHooks();
}
