#pragma once

#ifdef AIPUTIL_EXPORTS
#define AIPUTIL_API __declspec(dllexport)
#else
#define AIPUTIL_API __declspec(dllimport)
#endif

#include <ScriptUtils.h>

namespace AIPUtils
{
    // Handles setting a plan for the new AIP system.
    AIPUTIL_API void SetPlan(const char* aipCfg, const int teamNum);
    
    // Runs each game turn.
    AIPUTIL_API void Execute();
    
    // Adds the newHandle to a list of tracked GameObjects for the AIP to monitor.
    AIPUTIL_API void AddObject(Handle newHandle);
    
    // Removes the deadHandle from a list of tracked GameObjects for the AIP to monitor.
    AIPUTIL_API void RemoveObject(Handle deadHandle);
}