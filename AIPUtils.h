#pragma once

#ifdef AIPUTIL_EXPORTS
#define AIPUTIL_API __declspec(dllexport)
#else
#define AIPUTIL_API __declspec(dllimport)
#endif

#include <ScriptUtils.h>

// Import table from the game, defined here, declared in ScriptUtils.h, note that the time field will always be 0
// for some reason, if you want the true time value use misnExport.misnImport->time
MisnImport misnImport{};

// Export table from the DLL, it's not defined in the function block because it needs
// to stay in scope for the duration of the game.
MisnExport misnExport{};

namespace AIPUtils
{
    // Initial setup for the AIP DLL so ScriptUtils works.
    AIPUTIL_API void Setup();

    // Handles setting a plan for the new AIP system.
    AIPUTIL_API void SetPlan(const char* aipCfg, const int teamNum);
    
    // Runs each game turn.
    AIPUTIL_API void Execute();
    
    // Adds the newHandle to a list of tracked GameObjects for the AIP to monitor.
    AIPUTIL_API void AddObject(Handle newHandle);
    
    // Removes the deadHandle from a list of tracked GameObjects for the AIP to monitor.
    AIPUTIL_API void RemoveObject(Handle deadHandle);
}