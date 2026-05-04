#pragma once

#include <ScriptUtils.h>
#include "AIPGlobals.h"
#include "AIPIfCondition.h"


class AIPSchedPlan
{
public:
    int BuildId;
    Handle BuildHandle;

    int Team;
    int Priority;
    AIPIfCondition conditions[AIPGlobals::MAX_PLAN_CONDITIONS];
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

    static int PlanTimes[AIPGlobals::MAX_WORLDS];
    static int TargetUpdateTimes[AIPGlobals::MAX_WORLDS];

    // TODO: Update this to use the team numbers so maps with multiple AIPs can have individual staging paths.
    static Vector StagePoints[AIPGlobals::MAX_STAGE_POINTS];
    static int StagePointCount;

    static void ClearPlans(int team);
    static void LoadPlans(const char* cfg, int team);
    static void ReleaseUnits(int team, int priority);
    static void ReleaseFinishedUnits(int team, int priority);

    AIPSchedPlan(int team);
    virtual ~AIPSchedPlan() = default;

    virtual void Init(unsigned long cfg, unsigned long section) = 0;
    virtual void Done() = 0;

    virtual bool Execute() = 0;

    virtual void ResetTries() = 0;
    virtual const char* GetPlanName() = 0;
    virtual void ReleaseHandle(void* plan, int handle) = 0;

    virtual void Load()
    {
    }

    virtual void Save()
    {
    }

    virtual void PostLoad()
    {
    }

    virtual void ClaimTargets() = 0;
    virtual void ClaimDefendees() = 0;
    virtual void ReleaseUnits() = 0;
    virtual void ReleaseFinishedUnits() = 0;

    // Use Handle instead of Craft as we don't have access to the Craft object.
    Handle FindConstructionRig();

    void InitBuild();
    void StartBuild();
    void CheckBuild();

    bool ScrapSatisfied(const int curScrap, const int curTotalScrap) const;
    bool PowerSatisfied(const int curPower, const int curTotalPower) const;
    bool HisScrapSatisfied(const int curScrap, const int curTotalScrap) const;
    bool HisPowerSatisfied(const int curPower, const int curTotalPower) const;

    static AIPGlobals::PriorityGroup& FindGroup(int team, int priority);

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

    static Handle* FindScrap(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start);
    static Handle* FindIdleUnit(int team, const char* objClass, bool checkProvides,
                                const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);
    static Handle* FindIdleUnit(int team, const char* objClass, float ammoRatio, bool checkProvides,
                                const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);

    static void CommitBuild(int team, int slot);

    static Handle FindNearestTarget(int team, const char* targetClass, const Vector& start, bool checkProvides);
    static Handle FIndNearestDeposit(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start);

    static bool NeedsAmmo(Handle me, float threshold);
    
    static Handle* GetLiveObject(Handle me, int checkTeam = -1);
    static int CountMyUnits(int team, const char* objClass);
    static int CountMyUnits(int team, const char* objClass, float ammoRatio);

    // This is likely going to be the method from VT's Exus.
    static bool GoodSpot(int team, const char* objClass, Vector& pos, const Vector& front);
    static void GetClassAndUpgradeCRCs(const char* objClass, AIPGlobals::ObjClassCRCList& list);
    static bool CanBuild(Handle* builder, const char* itemClass, int priority);

    static void ClaimTarget(Handle* target, int team);
    static void UpdateTargets(int team);
    static void ClaimDefendee(Handle* target, int team);
    static void SubMultiPlayerClass(const char* objClass);

    static bool UpdateBuild(AIPGlobals::BuildRequest& req, bool checkProvides);
    static void UpdateBuilds(bool checkProvides);

    static void GetBuildStatus(int& buildId, Handle& handle, int priority);

    static void CancelBuild(const AIPGlobals::BuildRequest* request);
    static void CancelBuild(int buildId);
    static void CancelAllBuilds(int team);

    static AIPGlobals::TTB_RESULT TryToBuild(int team, int slot, const char* objClass, int priority);
    static bool WaitBuild(int team, int slot, const char* objClass, Handle& handle, bool checkProvides);

    static bool PlayerControlled(int team);
    static bool FindBuildSpot(int team, const char* objClass, const Vector& start, Vector& pos, Vector& dir);

    static void ReleasePlanHandle(void* plan, Handle* handle);

    static bool AreEnemiesNear(int team, Handle* target, float searchRadius);

    static bool Closer(Handle* firstHandle, Handle* secondHandle, int team);
    static bool CanShoot(Vector& pos, Handle* object);
};
