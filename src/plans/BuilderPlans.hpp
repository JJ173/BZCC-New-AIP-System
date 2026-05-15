#pragma once

#include "AIPSchedPlan.hpp"

class BuilderPlan : public AIPSchedPlan
{
public:
    enum BuildState : uint8_t
    {
        BUILD_CHECK,
        BUILD_FINDSPOT,
        BUILD_SUBMIT,
        BUILD_COMMIT,
        BUILD_WAIT
    };

    char BuildConsClass[ODF_MAX_LEN] = {};

    struct BuildInfo
    {
        char ObjClass[64] = {};
        int Count = 0;
        BuildState BuildState = BUILD_CHECK;
        int NeedsPower = 0;
    };

    std::vector<BuildInfo> BuildList;

    BuilderPlan(int team, TeamOverwatch* myTeamOverwatch) : AIPSchedPlan(team, myTeamOverwatch)
    {
        BuildConsClass[0] = 0;
    }

    void Init(std::string cfg, std::string section) override;
    bool Execute() override;

    virtual bool Execute(BuildInfo& info)
    {
        return true;
    }

    void Done() override
    {
        // Units handled by this plan get released by CancelAllBuilds()
    }
};

class BuildMinimums : public BuilderPlan
{
public:
    BuildMinimums(int team, TeamOverwatch* myTeamOverwatch) : BuilderPlan(team, myTeamOverwatch)
    {

    }

    const char* GetPlanName() override
    {
        return "BuildMinimums";
    }

    void ResetTries() override
    {

    }

    void ReleaseHandle(void* pThis, int handle) override
    {
    }

    void Init(std::string cfg, std::string section) override;
    bool Execute(BuildInfo& buildInfo) override;
};

class BuildLoop : public BuilderPlan
{
public:
    BuildLoop(int team, TeamOverwatch* myTeamOverwatch) : BuilderPlan(team, myTeamOverwatch)
    {
        Outer = 0;
        Inner = 0;
        Loops = 0;
        MaxLoops = 0;
        LoopDelay = 0;
        DelayActive = false;
        TimeOut = 0;
    }

    const char* GetPlanName() override
    {
        return "BuildLoop";
    }

    int Outer;
    int Inner;
    int Loops;
    int MaxLoops;
    int LoopDelay;
    bool DelayActive;
    long TimeOut;

    void ResetTries() override
    {

    }

    void ReleaseHandle(void* pThis, int handle) override
    {
    }

    void Init(std::string cfg, std::string section) override;

    bool Execute(BuildInfo& info) override;
    bool Execute(void) override;
};
