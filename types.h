#pragma once

class camBaseCameraMetadata
{
public:
	virtual ~camBaseCameraMetadata() = 0;
	virtual uintptr_t getMetadataFunc() = 0;
};

class camCinematicMountedCameraMetadata : public camBaseCameraMetadata
{
public:
	char pad0[0x138]; //0x8-0x140
	void *unkCameraMetadataCollection; //0x140-0x148
	char pad1[0x220]; //0x148-0x368
	float minPitch; //0x368-0x36C
	float maxPitch; //0x36C-0x370
};

class camAimCameraMetadata : public camBaseCameraMetadata { };

class camFirstPersonAimCameraMetadata : public camAimCameraMetadata { };

class camFirstPersonShooterCameraMetadata : public camAimCameraMetadata
{
public:
	char pad0[0x4C];
	float minPitch; //0x54-0x58
	float maxPitch; //0x58-0x5C
	char pad1[0x2A4];
	float ladderClimbMinPitch; //0x300
	float ladderClimbMaxPitch; //0x304
};

struct camCinematicMountedCamera
{
	void *vfTable; //0x0-0x8
	void *unk; //0x8-0x10
	camCinematicMountedCameraMetadata *unkMetadata; //0x10-0x18
	int iAliveDuration; //0x18 = 0x1C
	char pad0[0x4]; //0x1C-0x20
	char pad1[0x1F0]; //0x20-0x210
	void *baseEntity; //0x210-0x218 (CAutomobile)
	char pad2[0x1D8];
	float minPitch; //0x3F0
	float maxPitch;
	float minYaw;
	float maxYaw;
	char pad3[0x1C];
	float horizontalLookRot; //0x41C
	char pad4[0x10]; //0x41C-0x42C
	float verticalLookRot; //0x42C-0x430
};

struct camFirstPersonShooterCamera
{
	void *vfTable; //0x0-0x8
	void *unk; //0x8-0x10
	camFirstPersonShooterCameraMetadata *unkMetadata; //0x10-0x18
	int iActiveDuration; //0x18 = 0x1C
	char pad0[0x4]; //0x1C-0x20
	char pad1[0x1F0]; //0x20-0x210
	void *baseEntity; //0x210-0x218 (CPed)
	char pad2[0x48]; //0x218
	float unkMin; //0x260
	float unkMax; //0x264
	float pitchMin; //0x268
	float pitchMax; //0x26C
	char pad3[0x138]; //0x300
	float climbingPitchMin; //0x438 //nop 0x1402EA7AE
	float climbingPitchMax; //0x43C
	char pad4[0xD4]; //0x500
	float ladderPitch; //0x5D4
	float ladderYaw; //0x5D8
};