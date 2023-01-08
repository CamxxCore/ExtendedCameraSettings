#pragma once

#include <xmmintrin.h>

typedef __m128 CVector;

#pragma pack(push, 1)
struct CMatrix {
    CVector right;
    CVector forward;
    CVector up;
    CVector pos;
};
#pragma pack(pop)

namespace rage {
template<typename T>
class pgCollection {
  public:
    T*			m_data;
    UINT16		m_count;
    UINT16		m_size;

    T first() {
        return m_data[0];
    }

    T* begin() {
        return m_data;
    }

    T* end() {
        return (m_data + m_count);
    }

    T* at(UINT16 index) {
        return &m_data[index];
    }
};
}

enum eDynamicMenuAction {
    Slider = 0,
    Toggle = 1,
    AimMode = 3,
    GamepadLayout = 4,
    LowMedHi = 5,
    AudioOutput = 8,
    FadeRadio = 10,
    SelfRadioMode = 11,
    OffOnBlips = 13,
    SimpleComplex = 14,
    Language = 15,
    LowHi = 16
};

struct UIMenuItem {
    UIMenuItem() {
        memset(this, 0x0, sizeof(UIMenuItem));
    }

    int menuIndex; // 0x0-0x4
    unsigned int textHash; //0x4-0x8
    void * unkPtr; //0x8-0x10
    short unkCount; //0x10-0x12
    short unkCount1; //0x12-0x14
    char pad0[0x4]; //0x14-0x18
    unsigned char settingId; // 0x18-0x19 Index of the setting that is changed when this item is invoked
    char actionType; //0x19-0x1A i.e. slider, on_off etc.
    char type; //0x1A-0x1B 1 = default button, 2 = dynamic 8 = immutable text block
    char stateFlags; //0x1B-0x1C // 1 = disabled
    char pad1[0x4];
}; //sizeof=0x20

struct UIMenu {
    void * cmenuInstance; //0x0-0x8
    UIMenuItem * items; //0x8-0x10
    short itemCount; //0x10-0x12
    short maxItems; //0x12-0x14
    char pad0[0x4]; //0x14-0x18
    void * unk; //0x18-0x20
    char pad1[0x10]; //0x20-0x30
    const char * szMenuName; //0x30-0x38
    char pad2[0x8]; //0x38-0x40
    int menuId; //0x40-0x44
    int unk1; //0x44-0x48
    short unkFlag; //0x48-0x4A
    short scrollFlags; //0x4A-0x4C
    char pad[0x4];
}; //sizeof=0x50

class camBaseObjectMetadata {
  public:
    virtual ~camBaseObjectMetadata() = 0;
    virtual uintptr_t getPsoStruct() = 0;
    virtual unsigned int getBaseHash() = 0;
    unsigned int hashKey;
};

struct camMetadataRef {
    unsigned int nameHash;
    int unk;
    camBaseObjectMetadata * pData; //0x8-0x10
    camMetadataRef * pNext; //0x10-0x18
    char pad0[0x8]; //0x10-0x20
};

class camBaseCameraMetadata : public camBaseObjectMetadata {
  public:
    virtual ~camBaseCameraMetadata() = 0;
    virtual __int64 getMetadataFunc() = 0;
    unsigned int hashName;
};

struct camBaseCamera {
    char pad0[0x210];
    void * baseEntity; // 0x210-0x218
    char pad1[0x18];
    camBaseCameraMetadata * unkMetadata; //0x230
    char pad2[0x308];
    camBaseCameraMetadata * metadata; //0x540
};

struct camBaseDirector {
    char pad0[0x2C0];
    camBaseCamera * activeCamera; //0x2C0-0x2C8
    camBaseCamera * previousCamera; //0x2C8-0x2D0
    char pad1[0x108];
    void * ped; //0x3D8-0x3E0
    void * vehicle; //0x3E0-0x3E8
};

struct camCinematicMountedCameraMetadataLeadingLookSettings {
    char pad0[0x30];
};

struct camThirdPersonCameraMetadataPivotOverBoundingBoxSettings {
    void ** vfTable; //0x0-0x8

};

struct camFirstPersonShooterCameraMetadataRelativeAttachOrientationSettings {
    char pad0[0x28];
    float horizontalOrientation; //0x28-0x2C
    float verticalOrientation; //0x2C-0x30
    float camFrameOrientation; //0x30-0x34
    char pad1[0x3C];
}; //sizeof=0x70

struct camThirdPersonCameraMetadataBasePivotPosition {
    char pad0[0x8];
    bool bDisablePitchRotationZooming; // whether to disable camera pitch- relative zooming when pivoting around the entity.
    char pad1[0x3]; //0x9-0xC
    float fHighPitchZoomOutScale; //0xC-0x10 scale of zooming-out that occurs when the camera is pitched down looking at the vedhicle
};

struct camThirdPersonCameraMetadataVehicleOnTopOfVehicleCollisionSettings {
    char pad0[0x1C];
    float fLeftRotationClampLimit; //0x1C-0x20
    float fRightRotationClampLimit; //0x20-0x24
    float fDownRotationClampLimit; //0x20-0x24
    float fUpRotationClampLimit; //0x20-0x24
};


class camFollowPedCameraMetadata : public camBaseCameraMetadata {
  public:
    //(0x0-0x8 vftable)
    char pad0[0x5C]; //0x8-0x64
    bool bLerpCameraPivot; // if true, the camera will use linear interpolation when setting pivot values defined in 'camThirdPersonCameraMetadataBasePivotPosition'.
    char pad1[0xA8];
    bool bBoundingBoxRelativePivotIgnored; //0xB0-0xB1 whether to ignore bounding box dimensions when pivoting around the entity.
    char pad2[0x18F]; //0xB1-0x240
    camThirdPersonCameraMetadataVehicleOnTopOfVehicleCollisionSettings m_ontopOfVehicleCollisionSettings;
};

class camCinematicMountedCameraMetadata : public camBaseCameraMetadata {
  public:
    char pad0[0x54]; //0x8-0x58
    float fieldOfView; //0x54-0x58
    float zNear; //0x58-0x5C
    char pad1[0x108]; //0x5C-0x165
    bool disableUpdate; //0x164-0x165
    bool disableFixups; //0x165-0x166
    char pad3[0x182]; //0x166-0x368
    float fMinVehicleSpeedForCorrection; //0x2E8-0x2EC //ref 1402F2ADC
    char pad4[0x8];
    float fLeftLeadingLookCorrectionMultiplier; //0x2F4 left-side lerp speed when the vehicle starts moving.
    float fRightLeadingLookCorrectionMultiplier; //0x2F8 right-side lerp speed when the vehicle starts moving.
    float fLeadingLookCorrectionDurationMultiplier; //0x2FC lerp speed of the camera when vehicle is moving (lerp towards front)
    float fLeftLeadingLookCorrectionSpeed; //0x300-0x304 //ref 1402F2B8F how quick the correction will be when lerping from the left
    float fRightLeadingLookCorrectionSpeed; //0x304-0x308 //ref 1402F2B97 how quick the correction will be when lerping from the right
    char pad5[0x10]; // 0x308-0x318
    camCinematicMountedCameraMetadataLeadingLookSettings leadingLookSettings; //0x318-0x348
    float fMinPitchOverride; //0x348-0x34C
    float fMaxPitchOverride; //0x34C-0x350
    char pad6[0x5C]; // 0x308-0x360
    bool bDisableUpdateUnk; //0x360 vehicle shadow rendering?
    bool bDrawParticle; //0x361 seems to draw "floater" particles in front of the camera
    bool bShowReticule; //0x362
    bool bDisableHeadMesh; //0x363
    bool bDisableBodyMesh; //0x364
    char pad7[0x4]; //0x364-0x368
    float fMinPitch; //0x368-0x36C
    float fMaxPitch; //0x36C-0x370
};

//14028CC7C jmp = nop entering water adjustment

class camAimCameraMetadata : public camBaseCameraMetadata { };

class camFirstPersonAimCameraMetadata : public camAimCameraMetadata { };

class camFirstPersonShooterCameraMetadata : public camAimCameraMetadata {
  public:
    unsigned int metadataHash; //0x8-0xC
    char pad0[0x14]; //0xC-0x20
    unsigned int unkHash; //0x20-0x24
    float fieldOfView; //0x24-0x28
    float zNear; //0x28-0x2C
    char pad1[0x14]; //0x2C-0x40
    CVector entityRelativeOffset; //0x40-0x4C // camera offset relative to entity
    unsigned int unkHash1;
    float fMinPitch; //0x54-0x58
    float fMaxPitch; //0x58-0x5C
    float fMinYaw; //0x5C-0x60 // doesn't make any difference with first person camera
    float fMaxYaw; //0x60-0x64
    char pad2[0xC];
    bool bDisableBodyMesh; //0x70-0x71
    bool bDisableHeadMesh; //0x71-0x72
    bool bShowReticule; //0x72-0x73
    bool bUseEntityRelativeOffset; //0x73-0x74
    char pad3[0x5C];
    float fHorizontalRotationPivotScalar; //0xD0 scale at which the camera will pivot horizontally around its fixed point (relative to pitch) best at 0.120
    float fVerticalRotationPivotScalar; //0xD4 scale at which the camera will pivot forward and downward around its fixed point (relative to pitch) best at 0.0
    float fForwardRotationPivotScalar; //0xD8 scale at which the camera will pivot forward and upward around its fixed point (relative to pitch) best at 0.090
    char pad4[0x3C]; //0xD8-0x118
    float fWorldClipMinHeight; //0x118-0x11C // maximum height the camera should adjust to when doing a raycast against the world to avoid clipping through objects.
    char pad5[0x94]; //0x11C-0x1B0
    camFirstPersonShooterCameraMetadataRelativeAttachOrientationSettings orientationSettings; //0x1B0-0x220
    char pad6[0xC]; //0x220-0x22C
    float fovAimLerpMaxExtent; // 0x22C-0x230 max extent of fov lerp relative to g_userFovScale
    float fovAimLerpMinExtent; // 0x230-0x234
    float fovAimLerpSpeedScale; // 0x234-0x238 speed of lerp when camera transitions to aiming mode
    char pad7[0xC8]; //0x220-0x300
    float fLadderClimbMinPitch; //0x300
    float fLadderClimbMaxPitch; //0x304
    char pad8[0x2D4]; //0x308-0x5DC
    float fCoverTaskRightRelativeCorrectionAngle; //0x5DC
    float fCoverTaskRightUnk; //0x5DC
    float fCoverTaskLeftRelativeCorrectionAngle; //0x5E0
    float fCoverTaskLeftUnk; //0x5E4
    char pad9[0x320];
    int iUnkCoverTaskVar; //0x624
};

struct camCinematicMountedCamera {
    void *vfTable; //0x0-0x8
    void *unk; //0x8-0x10
    camCinematicMountedCameraMetadata *unkMetadata; //0x10-0x18
    int iAliveDuration; //0x18 = 0x1C
    char pad0[0x5C]; //0x1C-0x78
    float depthOfField; //0x78-0x7C
    char pad1[0x194]; //0x7C-0x210
    void *baseEntity; //0x210-0x218 (CAutomobile)
    char pad2[0x1D8];
    float minPitch; //0x3F0
    float maxPitch; //0x3F4
    float minYaw; //0x3F8
    float maxYaw; //0x3FC
    char pad3[0x1C];
    float horizontalLookRot; //0x41C
    char pad4[0xC]; //0x420-0x42C
    float verticalLookRot; //0x42C-0x430
};

struct camFirstPersonShooterCamera {
    void *vfTable; //0x0-0x8
    void *unk; //0x8-0x10
    camFirstPersonShooterCameraMetadata *unkMetadata; //0x10-0x18
    int iActiveDuration; //0x18 = 0x1C
    char pad0[0x74]; //0x1C-0x90
    float fieldOfView; // set each game loop at 0x14030A8FE
    char pad1[0x15C]; //0x94-0x1F0
    void *camInterpolator; //0x1F0-0x1F8
    char pad2[0x18]; //0x94-0x210
    void *baseEntity; //0x210-0x218 (CPed)
    char pad3[0x18]; //0x218-0x230
    camFirstPersonShooterCameraMetadata * defaultMetadata; //0x230-0x238
    char pad4[0x28];
    float variableMinPitchOffset; //0x260 // offset changes depending on what the player is doing
    float variableMaxPitchOffset; //0x264
    float pitchMin; //0x268
    float pitchMax; //0x26C
    char pad5[0x138]; //0x300
    float climbingPitchMin; //0x438 //nop 0x1402EA7AE
    float climbingPitchMax; //0x43C
    char pad6[0x174]; //0x440-0x5B4
    float parentFieldOfView; //0x5B4-0x5B8
    char pad7[0x1C]; //0x5B8-0x5D4
    float ladderPitch; //0x5D4
    float ladderYaw; //0x5D8-0x5DC
    char pad8[0x1B1];
    char unkMovementFlags; // related to camera control in and out of cover
};

struct camThirdPersonFollowCamera {
    char pad0[0x390];
    CVector cameraSpeedOffset; //0x390-0x3A0 -offset used to adjust the camera distance behind the entity relative to its speed.
};
