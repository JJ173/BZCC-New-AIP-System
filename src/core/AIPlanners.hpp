#pragma once

#include <ExtraUtils.h>
#include <ScriptUtils.h>

#include "AIPGlobals.hpp"
#include "AIPSchedPlan.hpp"
#include "PriorityGroup.hpp"
#include "TeamOverwatch.hpp"

class AIPSchedPlan;

class AIPlanners
{
    TeamOverwatch TeamOverwatchByTeam[MAX_TEAMS];

    static int GroupSize(std::vector<std::unique_ptr<AIPSchedPlan>>* plans, int startSlot);
    static bool SchedCompare(const AIPSchedPlan* p1, const AIPSchedPlan* p2);
    static void ClearPlanItems(int team);
    
public:
    static void SetPlan(const char* aipCfg, int teamNum);
    static void ExecuteSchedPlans();
};
