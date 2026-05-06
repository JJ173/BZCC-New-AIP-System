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
        
    TeamOverwatch* thisTeam = &AIPGlobals::TeamOverwatchState[teamNum];

    if (aipCfg == nullptr || aipCfg[0] == '\0')
    {
        // TRACE((team, " *** Clearing for team %d. turnNow = %d World=%d", team, TimeManager::Get().GetTurnNow(), TimeManager::Get().GetTurnWorld()));
        exu2::PrintConsoleMessage("SetPlan received an empty AIP file. Team: {} is now free.", teamNum);
        ClearPlanItems(teamNum);
        thisTeam->ClearPlans();
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
    thisTeam->ClearPlans();
    
    strncpy_s(AIPGlobals::LastPlanFile[teamNum], aipCfg, _TRUNCATE);
    strncpy_s(AIPGlobals::LastAIPFileForDLL[teamNum], aipCfg, _TRUNCATE);
    
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
    
    if (!OpenODF(aipCfg))
    {
        exu2::PrintConsoleMessage("AIP ERROR: AIP file {} not found!", aipCfg);
        return;
    }

    thisTeam->Setup(teamNum);
    thisTeam->LoadTuning(aipCfg);
    thisTeam->FirstLoad();
    thisTeam->LoadMatches();
    thisTeam->LoadPlanNames();
    thisTeam->LoadScheduledPlans();

    /* TODO: Find an alternative for this logging for the AIP.
    TRACE((team, "\n\n\n***************************************************\nLoadPlans - cfg(%s) team(%d)\n", cfg, team));
    LOG_DIAG(("SetAIP - cfg(%s) team(%d)", cfg, team));
    if (schedLogs[team] != NULL)
    {
        fflush(schedLogs[team]);
    }
    */

    // TODO: Ask VT about "LuaManager" and see if it's available. If not, I think we can easily work around this
    // based on the context.

    // Need to see if PriorityGroups is used anywhere. Will keep it commented out for now.
    //priorityGroups[team].clear();
    //int group = 0;
    //while (true)
    //{
    //    int groupPriority;
    //    GroupType groupType;
    //    char field[ODF_MAX_LEN];
    //    char str[ODF_MAX_LEN];
    //    //
    //    ++group;
    //    sprintf_s(field, "groupPriority%d", group);
    //    field[sizeof(field) - 1] = 0;
    //    if (!ParameterDB::GetInt(cfgCrc, 0x3B8E7EE1 /* "Start" */, field, &groupPriority, 0))
    //        break;
    //    sprintf_s(field, "groupType%d", group);
    //    field[sizeof(field) - 1] = 0;
    //    ParameterDB::GetString(cfgCrc, 0x3B8E7EE1 /* "Start" */, field, sizeof(str), str, "loop");
    //    if (_stricmp(str, "loop") == 0)
    //        groupType = PRI_GRP_LOOP;
    //    else if (_stricmp(str, "select") == 0)
    //        groupType = PRI_GRP_SELECT;
    //    else
    //        groupType = PRI_GRP_NORMAL;
    //    PriorityGroup group(groupPriority, groupType);
    //    priorityGroups[team].push_back(group);
    //}

    // Clean up.
    CloseODF(aipCfg);

    std::sort(&AIPGlobals::SchedPlan[teamNum][0], &AIPGlobals::SchedPlan[teamNum][AIPGlobals::MAX_PLANS], SchedCompare);
    AIPGlobals::LastAIPTeam = teamNum;
}