#include "AIPlanners.h"

#include <ExtraUtils.h>
#include <ScriptUtils.h>

#include "AIPGlobals.h"
#include "AIPSchedPlan.h"

bool AIPlanners::SchedCompare(const AIPSchedPlan* p1, const AIPSchedPlan* p2)
{
    int priority1 = -1, priority2 = -1;
    
    if (p2 != nullptr)
    {
        priority2 = p2->Priority;
    }
    
    if (p1 != nullptr)
    {
        priority1 = p1->Priority;
    }
    
    return priority1 > priority2;
}

void AIPlanners::ClearPlanItems(int team)
{
    for (int i = 0; i < MAX_TEAMS; ++i)
    {
        if (team >= 0 && i != team)
        {
            continue;
        }

        AIPGlobals::AttackTargets[i].clear();
        AIPGlobals::DefendTargets[i].clear();
        AIPGlobals::PriorityGroups[i].clear();
        AIPGlobals::TGTCenters[i] = Vector {0.f, 0.f, 0.f};
        AIPGlobals::NormalGroups[i] = AIPGlobals::PriorityGroup(0, AIPGlobals::NORMAL);
        AIPGlobals::ConstructorReserveScrap[i] = 0;
        AIPGlobals::ConstructorReservePriority[i] = 0;
        AIPGlobals::ConstructorReserveTime[i] = 0;
    }
}

void AIPlanners::SetPlan(const char* aipCfg, const int teamNum)
{
    if (teamNum < 0 || teamNum >= MAX_TEAMS)
    {
        return;
    }
            
    if (GetCurWorld() != AIPGlobals::WORLD_LOCKSTEP)
    {
        PrintConsoleMessage("** Multiworld is on. Can't change AIPs from console **");
        return;
    }
        
    if (aipCfg == nullptr || aipCfg[0] == '\0')
    {
        // TRACE((team, " *** Clearing for team %d. turnNow = %d World=%d", team, TimeManager::Get().GetTurnNow(), TimeManager::Get().GetTurnWorld()));
        exu2::PrintConsoleMessage("SetPlan received an empty AIP file. Team: {} is now free.", teamNum);
        ClearPlanItems(teamNum);
        AIPSchedPlan::ClearPlans(teamNum);
        return;
    }
        
    if (strcmp(AIPGlobals::LastPlanFile[teamNum], aipCfg) == 0)
    {
        // TRACE((team, " *** NOT SETTING %s FOR %d -- already running!", cfg, team));
        exu2::PrintConsoleMessage("{} is already running for team {}.", aipCfg, teamNum);
        return;
    }
        
    exu2::PrintConsoleMessage("SetPlan({}, {})", aipCfg, teamNum);
        
    ClearPlanItems(teamNum);
    AIPSchedPlan::ClearPlans(teamNum);
    
    strncpy_s(AIPGlobals::LastPlanFile[teamNum], aipCfg, _TRUNCATE);
    strncpy_s(AIPGlobals::LastAIPFileForDLL[teamNum], aipCfg, _TRUNCATE);
    strncpy_s(AIPGlobals::SchedTeams[teamNum].AIPName, aipCfg, _TRUNCATE);
    
    if (AIPGlobals::AIPDebugTeam < 0 || AIPGlobals::AIPDebugTeam == teamNum)
    {
        strncpy_s(AIPGlobals::LastAIPFile, aipCfg, _TRUNCATE);
        
        for (size_t ii = 0; ii < strlen(aipCfg); ii++)
        {
            if (islower(AIPGlobals::LastAIPFile[ii]))
            {
                AIPGlobals::LastAIPFile[ii] = static_cast<char>(toupper(AIPGlobals::LastAIPFile[ii]));   
            }
        }
    }
    
    AIPSchedPlan::LoadPlans(aipCfg, teamNum);
    std::sort(&AIPGlobals::SchedPlan[teamNum][0], &AIPGlobals::SchedPlan[teamNum][AIPGlobals::MAX_PLANS], SchedCompare);
    AIPGlobals::LastAIPTeam = teamNum;
}