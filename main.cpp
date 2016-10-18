
#include "stdinc.h"

typedef uintptr_t (__cdecl* camBaseCameraMetadataForHash)(Hash*, camBaseCameraMetadata*);

camBaseCameraMetadataForHash g_camBaseCameraMetadataForHash;

eGameState *g_gameState;

typedef unsigned int offset_t;

std::map<const char*, offset_t> offsets;

CConfig config = CConfig("POVCameraUnlocker.ini");

void main()
{
	eGameVersion gameVer = getGameVersion();

	// invalid game version
	if (gameVer == eGameVersion::VER_UNK) return;

	// get pointer to game state..
	auto result = Pattern((BYTE*)"\x0F\x29\x74\x24\x00\x85\xDB", "xxxx?xx").get();

	g_gameState = reinterpret_cast<eGameState*>(*reinterpret_cast<int *>(result - 4) + result);
	
	// get pointer to function for grabbing/ setting camera metadata..
	result = Pattern((BYTE*)"\x8A\x41\x3C\x24\xF9", "xxxxx").get(29);

	g_camBaseCameraMetadataForHash =
		reinterpret_cast<camBaseCameraMetadataForHash>(*reinterpret_cast<int *>(result + 1) + (result + 5));

	// get offsets for game version..
	offsets.insert(std::make_pair("firstPersonShooterPitchMin", 84));
	offsets.insert(std::make_pair("firstPersonShooterPitchMax", 88));
	offsets.insert(std::make_pair("firstPersonShooterClimbingPitchMin", 768));
	offsets.insert(std::make_pair("firstPersonShooterClimbingPitchMax", 772));
	offsets.insert(std::make_pair("cinematicMountedPitchMin", gameVer < 21 ? 824 : 872));
	offsets.insert(std::make_pair("cinematicMountedPitchMax", gameVer < 21 ? 828 : 876));

	scriptMain();
}

uintptr_t getCamMetadata(Hash hash)
{
	return g_camBaseCameraMetadataForHash(&hash, 0);
}

uintptr_t getCamMetadata(char * cameraName)
{
	Hash hash = GAMEPLAY::GET_HASH_KEY(cameraName);

	return getCamMetadata(hash);
}

void patchMetadata()
{
	uintptr_t pResult;

#pragma region Patch on-foot cam

	auto fpsPitchMinOffset = offsets["firstPersonShooterPitchMin"];

	auto fpsPitchMaxOffset = offsets["firstPersonShooterPitchMax"];

	auto fpsLadderPitchMinOffset = offsets["firstPersonShooterClimbingPitchMin"];

	if ((pResult = getCamMetadata(0xA70102CA))) // not sure on the string. "something_pov_camera"..
	{
		*reinterpret_cast<float*>(pResult + fpsPitchMinOffset) = 
			config.get<float>("camFirstPersonShooterCamera", "MinPitch", -78.0f);

		*reinterpret_cast<float*>(pResult + fpsPitchMaxOffset) = 
			config.get<float>("camFirstPersonShooterCamera", "MaxPitch", 80.0f);;

		*reinterpret_cast<float*>(pResult + fpsLadderPitchMinOffset) = 
			config.get<float>("camFirstPersonShooterCamera", "ClimbingMinPitch", -200.0f);

		//*reinterpret_cast<float*>(pResult + fpsLadderPitchMaxOffset) = 
			//config.GetConfigSetting<float>("camFirstPersonShooterCamera", "ClimbingMaxPitch");
	}

#pragma endregion

#pragma region Patch Vehicle Cams

	auto mountedCameraPitchMinOffset = offsets["cinematicMountedPitchMin"];

	auto mountedCameraPitchMaxOffset = offsets["cinematicMountedPitchMax"];

	float minPitch = config.get<float>("camCinematicMountedCamera", "MinPitch", -30.0f);

	float maxPitch = config.get<float>("camCinematicMountedCamera", "MaxPitch", 30.0f);

	if ((pResult = getCamMetadata("default_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("default_pov_camera_no_reverse")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("reduced_near_clip_pov_camera")))
	{

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("default_pov_camera_lookaround_mid")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("plane_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("lazer_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("heli_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("bicycle_road_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("dinghy_pov_camera")))
	{

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("jetmax_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("jetski_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("squalo_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("tropic_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = minPitch;

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

	if ((pResult = getCamMetadata("bus_pov_camera")))
	{
		*reinterpret_cast<float*>(pResult + mountedCameraPitchMinOffset) = 24.0f; //custom

		*reinterpret_cast<float*>(pResult + mountedCameraPitchMaxOffset) = maxPitch;
	}

#pragma endregion
}

void notifyAboveMap(char* message)
{
	UI::_SET_NOTIFICATION_TEXT_ENTRY("CELL_EMAIL_BCON");
	UI::_ADD_TEXT_COMPONENT_STRING(message);
	UI::_DRAW_NOTIFICATION(0, 1);
}

void scriptMain()
{
	while (true)
	{
		if ((*g_gameState) == eGameState::Playing)
		{
			patchMetadata();
			break;
		}

		WAIT(0);
	}
}