#pragma once

#include "TeamOverwatch.h"

class AIPSchedPlan;

class AIPlanners
{
    TeamOverwatch TeamOverwatchByTeam[MAX_TEAMS];

    static bool SchedCompare(const AIPSchedPlan* p1, const AIPSchedPlan* p2);
    static void ClearPlanItems(int team);
    
public:
    static void SetPlan(const char* aipCfg, int teamNum);
};
