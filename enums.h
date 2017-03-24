#pragma once

enum eGameState
{
	Playing,
	Intro,
	Startup = 3,
	MainMenu = 5,
	Loading = 6
};

enum eMetadataHash : unsigned int
{
	eCamFollowPedCameraMetadata = 3759477553,
	eCamFirstPersonShooterCameraMetadata = 3837693093,
	eCamCinematicMountedCameraMetadata = 2185301869,
	eCamFollowVehicleCameraMetadata = 420909885
};
