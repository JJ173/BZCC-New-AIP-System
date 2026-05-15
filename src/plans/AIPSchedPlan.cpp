#include "AIPSchedPlan.hpp"
#include "TeamOverwatch.hpp"

// ==================================================
// Constructor
// ==================================================

AIPSchedPlan::AIPSchedPlan(int team, TeamOverwatch* myTeamOverwatch)
{
    MyTeamOverwatch = myTeamOverwatch;
    ProducerHandle = 0;
    Team = team;
    Priority = 0;
    BuildHandle = 0;
}

// ==================================================
// Build Methods
// ==================================================

bool AIPSchedPlan::CheckBuild()
{
    if (BuildId == 0)
    {
        return true;
    }

    UpdateBuildStatus();
    return BuildId == 0;
}

void AIPSchedPlan::StartBuild(const char* objOdf)
{
    if (BuildId != 0)
    {
        return;
    }

    BuildId = SubmitBuildRequest(objOdf);
    BuildHandle = 0;
}

int AIPSchedPlan::SubmitBuildRequest(const char* objOdf)
{
    Producer* producer = BuilderSlot(objOdf);
    if (producer == nullptr)
    {
        return 0;
    }

    ProducerHandle = producer->ProdHandle;

    BuildRequest request = {};
    request.Id = AIPGlobals::BuildId++;
    request.Team = Team;
    request.Cost = Utilities::GetODFValueFromChain<int>(objOdf, Constants::GameObjectClassString, "scrapCost");
    request.ObjOdf = objOdf;
    request.Stage = Enums::BUILD_TRY;
    request.AssignedProducer = producer;
    request.StartTime = 0;
    request.BuildHandle = 0;
    request.Priority = Priority;

    MyTeamOverwatch->AddBuildRequest(request);

    return request.Id;
}

// TODO: Check if this is needed when we start implementing multiple handles.
// A good plan could be to cycle the build queue so if nothing is idle for the current queue item, 
// we cycle it back to the end of the array and keep checking if things can be built to prevent stalling.
Producer* AIPSchedPlan::BuilderSlot(const char* objOdf)
{
    if (objOdf == nullptr)
    {
        return nullptr;
    }

    Producer* result = nullptr;
    std::vector<std::unique_ptr<Producer>>& teamProducers = *MyTeamOverwatch->GetProducers();

    for (size_t i = 0; teamProducers.size(); i++)
    {
        std::unique_ptr<Producer>& prod = teamProducers[i];

        if (!CanBuild(prod->ProdHandle))
        {
            continue;
        }

        if (prod->IsProducerBusy())
        {
            continue;
        }

        if (!prod->CanBuildItem(objOdf))
        {
            continue;
        }

        result = &*prod;
    }

    // Fall back, try to find Recycler.
    Producer* recy = MyTeamOverwatch->GetRecycler();

    if (recy == nullptr || !IsBuilding(recy->ProdHandle))
    {
        return nullptr;
    }

    return MyTeamOverwatch->GetRecycler();
}

void AIPSchedPlan::UpdateBuildStatus()
{
    std::vector<std::unique_ptr<BuildRequest>>& teamBuildRequests = *MyTeamOverwatch->GetBuildRequests();

    for (size_t i = 0; i < teamBuildRequests.size(); ) 
    {
        std::unique_ptr<BuildRequest>& request = teamBuildRequests[i];

        if (!request) 
        {
            ++i;
            continue;
        }

        bool shouldRemove = false;

        if (request->AssignedProducer->ProdHandle != ProducerHandle)
        {
            ++i;
            continue;
        }

        switch (request->Stage)
        {
        case Enums::BUILD_TRY:
            if (AIPGlobals::ConstructorReserveScrap[Team])
            {
                int objCost = GetActualScrapCost(request->BuildHandle);
                int teamScrap = GetScrap(Team);
                int maxScrap = GetMaxScrap(Team);
                int cReserve = AIPGlobals::ConstructorReserveScrap[Team];

                if (Priority > AIPGlobals::ConstructorReservePriority[Team])
                {
                    cReserve = 0;
                }

                if (objCost > maxScrap || teamScrap < cReserve + objCost)
                {
                    // Mark for removal, but don't return yet
                    shouldRemove = true;
                    BuildId = 0;
                    BuildHandle = 0;
                    // We do NOT return here! We want to check other requests.
                }
            }
            break;

        case Enums::BUILD_DONE:
            BuildId = 0;
            BuildHandle = request->BuildHandle;
            shouldRemove = true;
            break;
        }

        if (shouldRemove)
        {
            teamBuildRequests.erase(teamBuildRequests.begin() + i);
        }
        else
        {
            ++i;
        }

        if (teamBuildRequests.empty())
        {
            BuildId = 0;
            BuildHandle = 0;
        }
    }
}

void AIPSchedPlan::InitBuild()
{
}

void AIPSchedPlan::CancelBuild(const BuildRequest* request)
{
}

void AIPSchedPlan::CancelBuild(int buildId)
{
}

void AIPSchedPlan::CancelAllBuilds(int team)
{
}

// ==================================================
// Scrap Methods
// ==================================================

bool AIPSchedPlan::ScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    if (MinTtlScrap && curTotalScrap < MinTtlScrap)
    {
        return false;
    }

    if (MaxTtlScrap && curTotalScrap >= MaxTtlScrap)
    {
        return false;
    }

    if (MinCurScrap && curScrap < MinCurScrap)
    {
        return false;
    }

    if (MaxCurScrap && curScrap >= MaxCurScrap)
    {
        return false;
    }

    return true;
}

bool AIPSchedPlan::HisScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    if (HisMinTtlScrap && curTotalScrap < HisMinTtlScrap)
    {
        return false;
    }

    if (HisMaxTtlScrap && curTotalScrap >= HisMaxTtlScrap)
    {
        return false;
    }

    if (HisMinCurScrap && curScrap < HisMinCurScrap)
    {
        return false;
    }

    if (HisMaxTtlScrap && curScrap >= HisMaxTtlScrap)
    {
        return false;
    }

    return true;
}

Handle AIPSchedPlan::FindScrap(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    return 0;
}

Deposit* AIPSchedPlan::FindNearestDeposit(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    Deposit* deposit = nullptr;
    float bestDist = Constants::MAX_FLOAT;
    std::vector<Deposit>::iterator i;

    for (i = Objects::MapPools.begin(); i != Objects::MapPools.end(); ++i)
    {
        Deposit& depositIter = *i;

        if ((scavMask & depositIter.DepositClassMask) != depositIter.DepositClassMatch)
        {
            continue;
        }

        if ((depositIter.DepositClassProvides & scavMask) != scavMatch)
        {
            continue;
        }

        // Need to check about locked pool state when possible with Exus.
        //if (o->IsLocked(team))
        //{
        //    continue;
        //}

        //if (LockedBySomeTeam(o) && DeployedScavNear(o))
        //{
        //    continue;
        //}

        float dist = Utilities::GetDistance2DSquared(depositIter.pos, start);

        if (dist < bestDist)
        {
            bestDist = dist;
            deposit = &depositIter;
        }
    }

    return deposit;
}

// ==================================================
// Power Methods
// ==================================================

bool AIPSchedPlan::PowerSatisfied(const int curPower, const int curTotalPower) const
{
    if (MinTtlPower != Constants::IGNORE_POWER && curTotalPower < MinTtlPower)
    {
        return false;
    }

    if (MaxTtlPower != Constants::IGNORE_POWER && curTotalPower >= MaxTtlPower)
    {
        return false;
    }

    if (MinCurPower != Constants::IGNORE_POWER && curPower < MinCurPower)
    {
        return false;
    }

    if (MaxCurPower != Constants::IGNORE_POWER && curPower >= MaxCurPower)
    {
        return false;
    }

    return true;
}

bool AIPSchedPlan::HisPowerSatisfied(const int curPower, const int curTotalPower) const
{
    if (HisMinTtlPower != Constants::IGNORE_POWER && curTotalPower < HisMinTtlPower)
    {
        return false;
    }

    if (HisMaxTtlPower != Constants::IGNORE_POWER && curTotalPower >= HisMaxTtlPower)
    {
        return false;
    }

    if (HisMinCurPower != Constants::IGNORE_POWER && curPower < HisMinCurPower)
    {
        return false;
    }

    if (HisMaxCurPower != Constants::IGNORE_POWER && curPower >= HisMaxCurPower)
    {
        return false;
    }

    return true;
}

// ==================================================
// Gets / Finds
// ==================================================

Handle AIPSchedPlan::FindConstructionRig()
{
    return 0;
}

PriorityGroup& AIPSchedPlan::FindGroup(int team, int priority)
{
    std::vector<PriorityGroup>& list = AIPGlobals::PriorityGroups[team];
    std::vector<PriorityGroup>::iterator iter;

    for (iter = list.begin(); iter != list.end(); ++iter)
    {
        PriorityGroup& group = *iter;

        if (group.Priority == priority)
        {
            return group;
        }
    }

    return AIPGlobals::NormalGroups[team];
}

void AIPSchedPlan::FindBase(int team, Vector& location)
{
}

Handle AIPSchedPlan::FindNearestTarget(int team, const char* targetClass, const Vector& start, bool checkProvides)
{
    return 0;
}

int AIPSchedPlan::GetSeqNo(Handle me)
{
    return 0;
}

Handle AIPSchedPlan::GetLiveObject(Handle handle , int checkTeam)
{
    if (checkTeam == -1)
    {
        return (IsAlive(handle) ? handle : 0);
    }

    if (!IsAlive(handle))
    {
        return 0;
    }

    if (GetTeamNum(handle) != checkTeam)
    {
        return 0;
    }

    return handle;
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
