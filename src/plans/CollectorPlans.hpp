#pragma once

#include "AIPSchedPlan.hpp"

class CollectorPlan : public AIPSchedPlan
{
public:
    enum CollectState : uint8_t
    {
        FIND_RESOURCE,
        BUILD_SCAV,
        SEND_SCAV
    };

    char BuildScavClass[ODF_MAX_LEN];

    CollectState State = FIND_RESOURCE;
    Handle ScavHandle = 0;
    Deposit* Deposit;

    float MaxDist = 0.0f;

    int MaxTimes = 0;
    int TriedTimes = 0;
    bool CheckProvides = AIPGlobals::DefaultCheckProvides[Team];

    CollectorPlan(int team, TeamOverwatch* myTeamOverwatch) : AIPSchedPlan(team, myTeamOverwatch)
    {

    }

    void Init(std::string cfg, std::string section) override;

    void ResetTries() override
    {
        TriedTimes = 0;
    }

    void ReleaseHandle(void* pThis, int handle) override
    {
    }

    void Done() override
    {
        if (ScavHandle != 0)
        {
            Stop(ScavHandle, 0);
        }
    }
};

class CollectField : public CollectorPlan
{
public:
    CollectField(int team, TeamOverwatch* myTeamOverwatch) : CollectorPlan(team, myTeamOverwatch) {}

    const char* GetPlanName() override
    {
        return "CollectField";
    }

    void Init(std::string cfg, std::string section) override;
    bool Execute() override;
};

class CollectPool : public CollectorPlan
{
public:
    CollectPool(int team, TeamOverwatch* myTeamOverwatch) : CollectorPlan(team, myTeamOverwatch)
    {

    }

    const char* GetPlanName() override
    {
        return "CollectPool";
    }

    void Init(std::string cfg, std::string section) override;
    bool Execute() override;
};
