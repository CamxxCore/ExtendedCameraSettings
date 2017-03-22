#pragma once

typedef struct
{
	int settingId;
	int value;
} profileSetting_t;

enum eMetadataHash : unsigned int
{
	eCamFollowPedCameraMetadata = 3759477553,
	eCamFirstPersonShooterCameraMetadata = 3837693093,
	eCamCinematicMountedCameraMetadata = 2185301869,
	eCamFollowVehicleCameraMetadata = 420909885
};

inline unsigned int hashString(const char * str)
{
	return GAMEPLAY::GET_HASH_KEY(const_cast<char*>(str));
}

inline void notifyAboveMap(char* message)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("STRING");
	UI::_ADD_TEXT_COMPONENT_STRING(message);
	UI::_DRAW_NOTIFICATION(0, 1);
}

inline void showSubtitle(const char *  msg, int duration = 5000)
{
	UI::_SET_TEXT_ENTRY_2("CELL_EMAIL_BCON");

	const int maxStringLength = 99;

	char subStr[maxStringLength];

	for (int i = 0; i < strlen(msg); i += maxStringLength)
	{
		memcpy_s(subStr, sizeof(subStr), &msg[i], min(maxStringLength - 1, strlen(msg) - i));

		subStr[maxStringLength - 1] = '\0';

		UI::_ADD_TEXT_COMPONENT_STRING(subStr);
	}

	UI::_DRAW_SUBTITLE_TIMED(duration, 1);
}


