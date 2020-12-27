
#include "stdafx.h"

BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD  ul_reason_for_call,
                      LPVOID lpReserved
                     ) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        scriptRegister(hModule, scriptLoad);
        keyboardHandlerRegister(scriptKeyboardEvent);
        break;
    case DLL_PROCESS_DETACH:
        scriptUnload();
        scriptUnregister(scriptLoad);
        keyboardHandlerUnregister(scriptKeyboardEvent);
    }
    return TRUE;
}
