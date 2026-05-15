#pragma once

#include <memory>
#include <cstdint>
#include <vector>
#include <format>
#include <ranges>
#include <string>

#include "AIPGlobals.hpp"
#include "AIPIfCondition.hpp"
#include "BuildRequest.hpp"
#include "Producer.hpp"
#include "Objects.hpp"

class TeamOverwatch;

// ==================================================
// Base Class
// ==================================================

class AIPSchedPlan
{
public:
    TeamOverwatch* MyTeamOverwatch;

    Handle ProducerHandle;
    Handle BuildHandle;

    int BuildId;
    int Team;
    int Priority;
    AIPIfCondition conditions[Constants::MAX_PLAN_CONDITIONS];
    bool BuildIfNoIdle;
    bool ContinueEvenIfFail;

    int MinTtlScrap;
    int MaxTtlScrap;
    int MinCurScrap;
    int MaxCurScrap;

    int HisMinTtlScrap;
    int HisMaxTtlScrap;
    int HisMinCurScrap;
    int HisMaxCurScrap;

    int MinTtlPower;
    int MaxTtlPower;
    int MinCurPower;
    int MaxCurPower;

    int HisMinTtlPower;
    int HisMaxTtlPower;
    int HisMinCurPower;
    int HisMaxCurPower;

    // TODO: Add Logging here.
    // char PlanType[MAX_ODF_LENGTH];
    // char Section[MAX_ODF_LENGTH];

    int PlanTimes[Constants::MAX_WORLDS];
    int TargetUpdateTimes[Constants::MAX_WORLDS];

    // TODO: Update this to use the team numbers so maps with multiple AIPs can have individual staging paths.
    static Vector StagePoints[Constants::MAX_STAGE_POINTS];
    static int StagePointCount;

    static void ReleaseUnits(int team, int priority);
    static void ReleaseFinishedUnits(int team, int priority);

    AIPSchedPlan(int team, TeamOverwatch* myTeamOverwatch);
    virtual ~AIPSchedPlan() = default;

    virtual void Init(std::string cfg, std::string section) = 0;
    virtual void Done() = 0;

    virtual bool Execute() = 0;

    virtual void ResetTries() = 0;
    virtual const char* GetPlanName() = 0;
    virtual void ReleaseHandle(void* plan, int handle) = 0;

    //virtual void Load(ILoadSaveVisitor& visitor)
    //{
    //    visitor.in(&buildId, sizeof(buildId), "buildId");
    //    visitor.in(&buildHandle, sizeof(buildHandle), "buildHandle");
    //}

    //virtual void Save(ILoadSaveVisitor& visitor)
    //{
    //    visitor.out(&buildId, sizeof(buildId), "buildId");
    //    visitor.out(&buildHandle, sizeof(buildHandle), "buildHandle");
    //}

    //virtual void PostLoad(ILoadSaveVisitor& visitor)
    //{
    //    buildHandle = ConvertHandle(buildHandle);
    //}

    virtual void ClaimTargets() 
    {
    }

    virtual void ClaimDefendees()
    {
    }

    virtual void ReleaseUnits()
    {
    }

    virtual void ReleaseFinishedUnits()
    {
    };

    int SubmitBuildRequest(const char* objOdf);

    // ==================================================
    // Old Build Methods
    // ==================================================

    // Use Handle instead of Craft as we don't have access to the Craft object.
    Handle FindConstructionRig();

    void InitBuild();
    void StartBuild(const char* objOdf);
    bool CheckBuild();

    void UpdateBuildStatus();

    Producer* BuilderSlot(const char* objOdf);

    bool ScrapSatisfied(const int curScrap, const int curTotalScrap) const;
    bool PowerSatisfied(const int curPower, const int curTotalPower) const;
    bool HisScrapSatisfied(const int curScrap, const int curTotalScrap) const;
    bool HisPowerSatisfied(const int curPower, const int curTotalPower) const;

    bool DidConditionsFail();

    static PriorityGroup& FindGroup(int team, int priority);

    static bool IsBase(int team, int slot, Vector& location);
    static void FindBase(int team, Vector& location);

    static void UpgradeUnit(Handle me, Handle him);
    static void Collect(Handle me, Handle resource);

    // These can be replaced with a ScriptUtils call.
    // static void Attack(Craft *unit, GameObject *target);
    // static void Defend(Craft *unit, GameObject *resource);
    // static void Follow(Craft *unit, GameObject *leader);
    // static void Goto(Craft *unit, GameObject *target);
    // static void Goto(Craft *unit, Vector *target);
    // static void Hold(Craft *unit, Vector *target);
    // static void LookAt(Craft *unit, GameObject *target);
    // static void Service(Craft *unit, GameObject *wreck);
    // static void Idle(Craft *unit);
    // static void Stop(Craft *unit);
    // static void Wait(Craft *unit);

    static bool IsIdle(Handle me, bool LookAtIsIdle = false);
    static bool IsIdleRig(Handle me);

    static int GetSeqNo(Handle me);

    static Handle FindScrap(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start);
    static Handle FindIdleUnit(int team, const char* objClass, bool checkProvides,
                                const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);
    static Handle FindIdleUnit(int team, const char* objClass, float ammoRatio, bool checkProvides,
                                const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);

    static void CommitBuild(int team, int slot);

    static Handle FindNearestTarget(int team, const char* targetClass, const Vector& start, bool checkProvides);
    static Deposit* FindNearestDeposit(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start);

    static bool NeedsAmmo(Handle me, float threshold);
    
    static Handle GetLiveObject(Handle me, int checkTeam = -1);
    static int CountMyUnits(int team, const char* objClass);
    static int CountMyUnits(int team, const char* objClass, float ammoRatio);

    // This is likely going to be the method from VT's Exus.
    static bool GoodSpot(int team, const char* objClass, Vector& pos, const Vector& front);

    static void ClaimTarget(Handle* target, int team);
    static void UpdateTargets(int team);
    static void ClaimDefendee(Handle* target, int team);
    static void SubMultiPlayerClass(const char* objClass);

    static void CancelBuild(const BuildRequest* request);
    static void CancelBuild(int buildId);
    static void CancelAllBuilds(int team);

    static bool WaitBuild(int team, int slot, const char* objClass, Handle& handle, bool checkProvides);

    static bool PlayerControlled(int team);
    static bool FindBuildSpot(int team, const char* objClass, const Vector& start, Vector& pos, Vector& dir);

    static void ReleasePlanHandle(void* plan, Handle* handle);

    static bool AreEnemiesNear(int team, Handle* target, float searchRadius);

    static bool Closer(Handle* firstHandle, Handle* secondHandle, int team);
    static bool CanShoot(Vector& pos, Handle* object);
};

#include "BuilderPlans.hpp"
#include "CollectorPlans.hpp"