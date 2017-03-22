#pragma once

struct profileSettingsMgr
{
public:
	int get(int settingId);
	void registerNew(int settingId, int value);
	int indexForId(int settingId);
private:
	char pad[0x48];
}; //sizeof=0x48 (sub_1401B17EC)