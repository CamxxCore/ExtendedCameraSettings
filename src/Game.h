#pragma once

class Game {
    eGameState * m_pGameState = nullptr;
    rage::pgCollection<UIMenu> * m_pActiveMenuPool = nullptr;
    rage::pgCollection<camBaseDirector*> * m_pCamDirectorPool = nullptr;
    rage::pgCollection<camMetadataRef*> * m_pCamMetadataPool = nullptr;
  public:
    bool Initialize(int gameVersion);

    decltype(m_pGameState) GetGameState() const {
        return m_pGameState;
    }

    decltype(m_pActiveMenuPool) GetActiveMenuPool() const {
        return m_pActiveMenuPool;
    }

    decltype(m_pCamDirectorPool) GetCamDirectorPool() const {
        return m_pCamDirectorPool;
    }

    decltype(m_pCamMetadataPool) GetCamMetadataPool() const {
        return m_pCamMetadataPool;
    }
};

inline camBaseDirector* GetCamDirectorFromPool(unsigned long long * hashNameOrDef) {
    return ((camBaseDirector* (__fastcall*)(unsigned long long*))
            (*g_addresses.get("game"))["getCamDirectorFromPool"].addr)(hashNameOrDef);
}
