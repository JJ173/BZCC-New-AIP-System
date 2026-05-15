#pragma once

#include "Enums.hpp"

struct PriorityGroup
{
    int Priority;
    int Type;
    bool InProgress;
    int CurPlan;

    PriorityGroup(int priority, int groupType)
    {
        Priority = priority;
        Type = groupType;
        InProgress = false;
        CurPlan = 0;
    }

    PriorityGroup()
    {
        Priority = 0;
        Type = Enums::GroupType::NORMAL;
        InProgress = false;
        CurPlan = 0;
    }
};