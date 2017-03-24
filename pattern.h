#pragma once

#include <Psapi.h>

class Pattern sealed
{
public:
	Pattern(const BYTE* bMask, const char* szMask) : bMask(bMask), szMask(szMask) {}

	template <typename T>
	inline T get(int offset = 0)
	{
		MODULEINFO module = {};

		GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &module, sizeof(MODULEINFO));

		auto *address = reinterpret_cast<const char *>(module.lpBaseOfDll);

		auto address_end = address + module.SizeOfImage;

		for (; address < address_end; address++)
		{
			if (bCompare((BYTE*)(address), bMask, szMask))
			{
				return (T)(address + offset);
			}
		}

		return 0;
	}

	inline uintptr_t get(int offset = 0)
	{
		return get<uintptr_t>(offset);
	}

private:
	inline bool bCompare(const BYTE* pData, const BYTE* bMask, const char* szMask)
	{
		for (; *szMask; ++szMask, ++pData, ++bMask)
			if (*szMask == 'x' && *pData != *bMask)
				return 0;
		return (*szMask) == NULL;
	}

	const BYTE *bMask; const char* szMask;
};