// dllmain.cpp : Defines the entry point for the DLL application.
#include <ExtraUtils.h>
#include <ScriptUtils.h>

#include <Windows.h>
#include <delayimp.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);
        exu2::ProcessAttach();
        SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_APPLICATION_DIR |
            LOAD_LIBRARY_SEARCH_DEFAULT_DIRS |
            LOAD_LIBRARY_SEARCH_SYSTEM32 |
            LOAD_LIBRARY_SEARCH_USER_DIRS
        );
        break;
    case DLL_PROCESS_DETACH:
        exu2::ProcessDetach();
        if (GetModuleHandleW(L"NewAIPSystem.dll"))
        {
            __FUnloadDelayLoadedDLL2("NewAIPSystem.dll");
        }
        break;
    }
	
    return TRUE;
}

