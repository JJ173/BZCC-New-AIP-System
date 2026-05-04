#include "AIPUtils.h"

#include "AIPGlobals.h"
#include "AIPlanners.h"
#include <ExtraUtils.h>

namespace AIPUtils
{
    AIPUTIL_API void SetPlan(const char* aipCfg, const int teamNum)
    {
        AIPlanners::SetPlan(aipCfg, teamNum);
    }
    
    AIPUTIL_API void AddObject(Handle newHandle)
    {
        exu2::PrintConsoleMessage("Found a new handle: {}", newHandle);
    }
    
    AIPUTIL_API void RemoveObject(Handle newHandle)
    {
        exu2::PrintConsoleMessage("Removing a handle: {}", newHandle);
    }
}
