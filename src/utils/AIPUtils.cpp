#include "AIPUtils.hpp"

namespace AIPUtils
{
    AIPUTIL_API void Setup() 
    {
        const MisnExport* ex = exu2::GetMissionExport();

        if (ex && ex->misnImport)
        {
            misnImport = *ex->misnImport;
        }
    }

    AIPUTIL_API void SetPlan(const char* aipCfg, const int teamNum)
    {
        AIPlanners::SetPlan(aipCfg, teamNum);
    }
    
    AIPUTIL_API void AddObject(Handle newHandle)
    {
        // If for whatever reason we are given a dead handle, just return.
        if (newHandle == 0)
        {
            return;
        }

        char goClass[ODF_MAX_LEN];
        GetObjInfo(newHandle, Get_GOClass, goClass);

        if (strcmp(goClass, "CLASS_DEPOSIT") == 0)
        {
            Deposit deposit{};
            deposit.h = newHandle;
            deposit.pos = GetPosition(newHandle);
            Objects::MapPools.push_back(std::move(deposit));
            return;
        }

        // Can the object build stuff, if so, can we add it as a producer?
        if (CanBuild(newHandle) || Utilities::IsRecycler(goClass))
        {
            char objODF[ODF_MAX_LEN];
            GetObjInfo(newHandle, Get_ODF, objODF);

            for (int i = 0; i < MAX_TEAMS; ++i)
            {
                std::string resolvedClass(goClass);
                std::string resolvedOdf(objODF);

                AIPGlobals::TeamOverwatchState[i].AddProducer(newHandle, resolvedClass, resolvedOdf);
            }
        }
    }
    
    AIPUTIL_API void RemoveObject(Handle newHandle)
    {
        exu2::PrintConsoleMessage("Removing a handle: {}", newHandle);
    }

    AIPUTIL_API void Execute() 
    {
        AIPGlobals::CurrentGameTurn++;
        AIPlanners::ExecuteSchedPlans();
    }
}
