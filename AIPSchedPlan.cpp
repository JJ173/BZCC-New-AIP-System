#include "AIPSchedPlan.h"
#include <ExtraUtils.h>
#include "TeamOverwatch.h"

// ==================================================
// Constructor
// ==================================================

AIPSchedPlan::AIPSchedPlan(int team)
{
    BuildId = 0;
    BuildHandle = 0;
    Team = team;
    Priority = 0;
}

// ==================================================
// Build Methods
// ==================================================

int AIPSchedPlan::BuilderSlot(const char* objClass)
{
    if (objClass == nullptr)
    {
        return 0; 
    }

    // TODO: Revisit this to allow for multiple Factories, etc, to run with the AIP.
    for (int i = DLL_TEAM_SLOT_RECYCLER; i <= DLL_TEAM_SLOT_BASE9; ++i)
    {
        Handle producer = GetObjectByTeamSlot(Team, i);


    }

    return DLL_TEAM_SLOT_RECYCLER;
}

void AIPSchedPlan::InitBuild()
{
}

void AIPSchedPlan::StartBuild(char* objClass)
{
    if (BuildId != 0)
    {
        return;
    }

    BuildId = SubmitBuild(objClass, 0);
}

int AIPSchedPlan::SubmitBuild(const char* objClass, int priority)
{
    AIPGlobals::BuildRequest request;
    request.Id = BuildId++;
    request.Team = Team;
    request.ObjClass = objClass;
    request.Stage = 0;
    request.Slot = BuilderSlot(objClass);
    request.StartTime = 0;
    request.Handle = 0;
    request.Priority = priority;
    AIPGlobals::BuildRequests.push_back(std::move(request));
    return request.Handle;
}

void AIPSchedPlan::CheckBuild()
{
}

void AIPSchedPlan::CommitBuild(int team, int slot)
{
}

bool AIPSchedPlan::CanBuild(Handle* builder, const char* itemClass, int priority)
{
    return false;
}

bool AIPSchedPlan::UpdateBuild(AIPGlobals::BuildRequest& req, bool checkProvides)
{
    return false;
}

void AIPSchedPlan::UpdateBuilds(bool checkProvides)
{
}

void AIPSchedPlan::GetBuildStatus(int& buildId, Handle& handle, int priority)
{
}

void AIPSchedPlan::CancelBuild(const AIPGlobals::BuildRequest* request)
{
}

void AIPSchedPlan::CancelBuild(int buildId)
{
}

void AIPSchedPlan::CancelAllBuilds(int team)
{
}

AIPGlobals::TTB_RESULT AIPSchedPlan::TryToBuild(int team, int slot, const char* objClass, int priority)
{
    return AIPGlobals::TTB_RESULT::BADTEAM;
}

bool AIPSchedPlan::WaitBuild(int team, int slot, const char* objClass, Handle& handle, bool checkProvides)
{
    return false;
}

// ==================================================
// Scrap Methods
// ==================================================

bool AIPSchedPlan::ScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    return false;
}

bool AIPSchedPlan::HisScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    return false;
}

Handle* AIPSchedPlan::FindScrap(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    return nullptr;
}

// ==================================================
// Power Methods
// ==================================================

bool AIPSchedPlan::PowerSatisfied(const int curPower, const int curTotalPower) const
{
    return false;
}

bool AIPSchedPlan::HisPowerSatisfied(const int curPower, const int curTotalPower) const
{
    return false;
}

// ==================================================
// Gets / Finds
// ==================================================

Handle AIPSchedPlan::FindConstructionRig()
{
    return 0;
}

AIPGlobals::PriorityGroup& AIPSchedPlan::FindGroup(int team, int priority)
{
    return AIPGlobals::PriorityGroups[team][priority];
}

void AIPSchedPlan::FindBase(int team, Vector& location)
{
}

Handle* AIPSchedPlan::FindIdleUnit(int team, const char* objClass, bool checkProvides, const AIPGlobals::HandleList& excludeList,
    bool LookAtIsIdle)
{
    return nullptr;
}

Handle* AIPSchedPlan::FindIdleUnit(int team, const char* objClass, float ammoRatio, bool checkProvides,
    const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle)
{
    return nullptr;
}

Handle AIPSchedPlan::FindNearestTarget(int team, const char* targetClass, const Vector& start, bool checkProvides)
{
    return 0;
}

Handle AIPSchedPlan::FindNearestDeposit(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    return 0;
}

int AIPSchedPlan::GetSeqNo(Handle me)
{
    return 0;
}

Handle* AIPSchedPlan::GetLiveObject(Handle me, int checkTeam)
{
    return nullptr;
}

void AIPSchedPlan::GetClassAndUpgradeCRCs(const char* objClass, AIPGlobals::ObjClassCRCList& list)
{
}

// ==================================================
// Commands
// ==================================================

void AIPSchedPlan::UpgradeUnit(Handle me, Handle him)
{
}

void AIPSchedPlan::Collect(Handle me, Handle resource)
{
}

// ==================================================
// Idle Checks
// ==================================================

bool AIPSchedPlan::IsIdle(Handle me, bool LookAtIsIdle)
{
    return false;
}

bool AIPSchedPlan::IsIdleRig(Handle me)
{
    return false;
}

// ==================================================
// Plan Conditions
// ==================================================

bool AIPSchedPlan::DidConditionsFail()
{
    return false;
}

bool AIPSchedPlan::NeedsAmmo(Handle me, float threshold)
{
    return false;
}

bool AIPSchedPlan::IsBase(int team, int slot, Vector& location)
{
    return false;
}

bool AIPSchedPlan::GoodSpot(int team, const char* objClass, Vector& pos, const Vector& front)
{
    return false;
}

bool AIPSchedPlan::PlayerControlled(int team)
{
    return false;
}

bool AIPSchedPlan::AreEnemiesNear(int team, Handle* target, float searchRadius)
{
    return false;
}

bool AIPSchedPlan::Closer(Handle* firstHandle, Handle* secondHandle, int team)
{
    return false;
}

bool AIPSchedPlan::CanShoot(Vector& pos, Handle* object)
{
    return false;
}

// ==================================================
// Counts
// ==================================================

int AIPSchedPlan::CountMyUnits(int team, const char* objClass)
{
    return 0;
}

int AIPSchedPlan::CountMyUnits(int team, const char* objClass, float ammoRatio)
{
    return 0;
}

// ==================================================
// Claims / Releases
// ==================================================

void AIPSchedPlan::ClaimTarget(Handle* target, int team)
{
}

void AIPSchedPlan::UpdateTargets(int team)
{
}

void AIPSchedPlan::ClaimDefendee(Handle* target, int team)
{
}

void AIPSchedPlan::SubMultiPlayerClass(const char* objClass)
{
}

void AIPSchedPlan::ReleasePlanHandle(void* plan, Handle* handle)
{
}

void AIPSchedPlan::ReleaseUnits(int team, int priority)
{
}

void AIPSchedPlan::ReleaseFinishedUnits(int team, int priority)
{
}
