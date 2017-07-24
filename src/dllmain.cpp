
#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		scriptRegister(hModule, scriptMain);
		keyboardHandlerRegister(scriptKeyboardMessage);
		break;
	case DLL_PROCESS_DETACH:
		unload();
		scriptUnregister(scriptMain);
		keyboardHandlerUnregister(scriptKeyboardMessage);
	}
	return TRUE;
}
