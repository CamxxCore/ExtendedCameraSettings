#include "stdinc.h"

int profileSettingsMgr::get(int settingId)
{
	return ((int(__fastcall*)(profileSettingsMgr*, int))
		(*g_addresses.get("general"))["getProfileSetting"])(this, settingId);
}

int profileSettingsMgr::indexForId(int settingId)
{
	return ((int(__fastcall*)(profileSettingsMgr*, int))
		(*g_addresses.get("general"))["getProfileSettingIndex"])(this, settingId);
}

void profileSettingsMgr::registerNew(int settingId, int value)
{
	((void(__fastcall*)(profileSettingsMgr*, int, int))
		(*g_addresses.get("general"))["registerProfileSetting"])(this, settingId, value);
}
