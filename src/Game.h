#pragma once

class Game {
	eGameState * m_pGameState;
	rage::pgCollection<UIMenu> * m_pActiveMenuPool;
	rage::pgCollection<camBaseDirector*> * m_pCamDirectorPool;
	rage::pgCollection<camMetadataRef*> * m_pCamMetadataPool;	

public:
	bool Initialize();

	decltype(m_pGameState) GetGameState() const
	{
		return m_pGameState;
	}

	decltype(m_pActiveMenuPool) GetActiveMenuPool() const
	{
		return m_pActiveMenuPool;
	}

	decltype(m_pCamDirectorPool) GetCamDirectorPool() const
	{
		return m_pCamDirectorPool;
	}

	decltype(m_pCamMetadataPool) GetCamMetadataPool() const
	{
		return m_pCamMetadataPool;
	}
};

inline camBaseDirector* GetCamDirectorFromPool(unsigned int * hashName)
{
	return ((camBaseDirector* (__fastcall*)(unsigned int*))
		(*g_addresses.get("game"))["getCamDirectorFromPool"].addr)(hashName);
}
