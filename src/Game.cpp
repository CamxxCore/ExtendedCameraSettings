#include "stdafx.h"

AddressMgr g_addresses;

bool Game::Initialize(int gameVersion) {
    auto addresses = g_addresses.getOrCreate("game");

    #pragma region Game State

    // get pointer to game state..
    auto result = (uintptr_t)BytePattern(gameVersion < VER_1_0_2545_0_STEAM ? (BYTE*)"\x0F\x29\x74\x24\x00\x85\xDB" :
        (BYTE*)"\x0F\x29\x74\x24\x00\x85\xC0", "xxxx?xx").get().get();

    if (result) {
        LOG("Game state found at 0x%llX", result);
    }

    else {
        LOG("[ERROR] Failed to find game state");
        return false;
    }

    m_pGameState = reinterpret_cast<eGameState*>(*reinterpret_cast<int *>(result - 4) + result);

    #pragma endregion

    #pragma region getCamDirectorFromPool

    auto pattern = BytePattern((BYTE*)"\xE8\x00\x00\x00\x00\x48\x8D\x48\x30\xEB\x0B", "x????xxxxxx");

    result = (uintptr_t)pattern.get().getCall().get();

    if (result) {
        LOG("getCamDirectorFromPool() found at 0x%llX", result);
    }

    else {
        LOG("[ERROR] Failed to find getCamDirectorFromPool()");
        return false;
    }

    addresses->insert("getCamDirectorFromPool", result);

    #pragma endregion

    #pragma region Cam Metadata Pool

    result = (uintptr_t)BytePattern((BYTE*)"\x48\x8D\x94\x24\x40\x01\x00\x00\x48\x8D\x0D\x00\x00\x00\x00", "xxxxxxxxxxx????").get().getTargetRel7(8).get();

    if (result) {
        LOG("CamMetadataPool found at 0x%llX", result);
    }

    else {
        LOG("[ERROR] Failed to find CamMetadataPool");
        return false;
    }

    m_pCamMetadataPool = reinterpret_cast<rage::pgCollection<camMetadataRef*>*>(result);

    #pragma endregion

    #pragma region Active Menu Pool

    result = (uintptr_t)BytePattern((BYTE*)"\x0F\xB7\x54\x51\x00", "xxxx?").get().get();

    if (result) {
        LOG("ActiveMenuPool found at 0x%llX", result);
    }

    else {
        LOG("[ERROR] Failed to find ActiveMenuPool");
        return false;
    }

    m_pActiveMenuPool = reinterpret_cast<rage::pgCollection<UIMenu>*>(*reinterpret_cast<int *>(result - 4) + result);

    #pragma endregion

    return true;
}
