#include "AIPSchedPlan.h"

#include <ExtraUtils.h>

#include "TeamOverwatch.h"

AIPSchedPlan::AIPSchedPlan(int team)
{
    BuildId = 0;
    BuildHandle = 0;
    Team = team;
    Priority = 0;
}

Handle AIPSchedPlan::FindConstructionRig()
{
    return 0;
}

void AIPSchedPlan::InitBuild()
{
}

void AIPSchedPlan::StartBuild()
{
}

void AIPSchedPlan::CheckBuild()
{
}

bool AIPSchedPlan::ScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    return false;
}

bool AIPSchedPlan::PowerSatisfied(const int curPower, const int curTotalPower) const
{
    return false;
}

bool AIPSchedPlan::HisScrapSatisfied(const int curScrap, const int curTotalScrap) const
{
    return false;
}

bool AIPSchedPlan::HisPowerSatisfied(const int curPower, const int curTotalPower) const
{
    return false;
}

AIPGlobals::PriorityGroup& AIPSchedPlan::FindGroup(int team, int priority)
{
    return AIPGlobals::PriorityGroups[team][priority];
}

bool AIPSchedPlan::IsBase(int team, int slot, Vector& location)
{
    return false;
}

void AIPSchedPlan::FindBase(int team, Vector& location)
{
}

void AIPSchedPlan::UpgradeUnit(Handle me, Handle him)
{
}

void AIPSchedPlan::Collect(Handle me, Handle resource)
{
}

bool AIPSchedPlan::IsIdle(Handle me, bool LookAtIsIdle)
{
    return false;
}

bool AIPSchedPlan::IsIdleRig(Handle me)
{
    return false;
}

int AIPSchedPlan::GetSeqNo(Handle me)
{
    return 0;
}

Handle* AIPSchedPlan::FindScrap(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    return nullptr;
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

void AIPSchedPlan::CommitBuild(int team, int slot)
{
}

Handle AIPSchedPlan::FindNearestTarget(int team, const char* targetClass, const Vector& start, bool checkProvides)
{
    return 0;
}

Handle AIPSchedPlan::FIndNearestDeposit(int scavMask, int scavMatch, int scavProvides, int team, const Vector& start)
{
    return 0;
}

bool AIPSchedPlan::NeedsAmmo(Handle me, float threshold)
{
    return false;
}

Handle* AIPSchedPlan::GetLiveObject(Handle me, int checkTeam)
{
    return nullptr;
}

int AIPSchedPlan::CountMyUnits(int team, const char* objClass)
{
    return 0;
}

int AIPSchedPlan::CountMyUnits(int team, const char* objClass, float ammoRatio)
{
    return 0;
}

bool AIPSchedPlan::GoodSpot(int team, const char* objClass, Vector& pos, const Vector& front)
{
    return false;
}

void AIPSchedPlan::GetClassAndUpgradeCRCs(const char* objClass, AIPGlobals::ObjClassCRCList& list)
{
}

bool AIPSchedPlan::CanBuild(Handle* builder, const char* itemClass, int priority)
{
    return false;
}

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

bool AIPSchedPlan::PlayerControlled(int team)
{
    return false;
}

bool AIPSchedPlan::FindBuildSpot(int team, const char* objOdf, const Vector& start, Vector& pos, Vector& dir)
{
    Vector points[100];
    {
        constexpr float delta = 8; // What is 8? Meters per grid, or what? Magic numbers, man, they suck.
        const float x0 = start.x - 5 * delta;
        const float z0 = start.z - 5 * delta;
        
        int n = 0;
        
        for (int i = 0; i < 10; i++)
        {
            for (int j = 0; j < 10; j++)
            {
                points[n].x = x0 + static_cast<float>(i) * delta;
                points[n].y = 0;
                points[n].z = z0 + static_cast<float>(j) * delta;
                ++n;
            }       
        }
    }
    
    AIPGlobals::CompareOrigin = start;
    std::sort(&points[0], &points[99], AIPGlobalUtils::ComparePoints);
    {
        Vector fronts[4] = 
        {
            Vector {1.0f, 0.f, 0.f},
            Vector {0.f, 0.f, 1.0f},
            Vector {-1.0f, 0.f, 0.f},
            Vector {0.f, 0.0f, -1.0f}
        };
        
        for (const auto point : points)
        {
            for (auto front : fronts)
            {
                pos = point;
                
                if (exu2::IsTerrainBuildable(team, objOdf, pos, front) == exu2::TerrainQueryResult::BUILDABLE)
                {
                    dir = front;
                    return true;
                }
            }
        }
    }
    
    pos = start;
    dir = Vector {0.0f, 0.0f, 1.0f};
    return false;
}

void AIPSchedPlan::ReleasePlanHandle(void* plan, Handle* handle)
{
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

void AIPSchedPlan::ClearPlans(int team)
{
}

void AIPSchedPlan::LoadPlans(const char* cfg, int team)
{
    TeamOverwatch* thisTeam = &AIPGlobals::TeamOverwatchState[team];
    
    /* TODO: Find an alternative for this logging for the AIP.
    TRACE((team, "\n\n\n***************************************************\nLoadPlans - cfg(%s) team(%d)\n", cfg, team));
    LOG_DIAG(("SetAIP - cfg(%s) team(%d)", cfg, team));
    if (schedLogs[team] != NULL)
    {
        fflush(schedLogs[team]);
    }
    */
    
    if (!OpenODF(cfg))
    {
        exu2::PrintConsoleMessage("ERROR: AIP file {} not found!", cfg);
        return;
    }
    
    thisTeam->Setup(team);
    thisTeam->LoadTuning(cfg);
    
    // TODO: Ask VT about "LuaManager" and see if it's available. If not, I think we can easily work around this
    // based on the context.
    
    // Check what "First Load" is from the SaveType. Will need to talk to VT again about `ILoadSaveVisitor`.
    // bool bFirstLoad = (ILoadSaveVisitor::GetCurrentSaveType() != ST_SAVE) && (ILoadSaveVisitor::GetCurrentSaveType() != ST_JOIN);
    bool firstLoad = true;
    
    if (AIPGlobals::SchedPlan[team][0] == nullptr && firstLoad)
    {
        int teamScrap = GetScrap(team);
        int scrapCount = 0;
        GetODFInt(cfg, "Start", "ScrapCount", &scrapCount, teamScrap);
        
        if (teamScrap < scrapCount)
        {
            SetScrap(team, scrapCount);
        }
        
        Handle recycler = thisTeam->GetRecycler();
        
        // TODO: Ensure this team isn't a player team, otherwise we can take direct control of the Recycler.
        if (IsAlive(recycler) && !IsBuilding(recycler))
        {
            Vector dropPoint{0.0f, 0.0f, 0.0f}, dropDir{0.0f, 0.0f, 0.0f};
            
            bool findSpot = true;
            char baseLoc[ODF_MAX_LEN];
            
            if (GetODFString(cfg, "Start", "BaseLoc", sizeof(baseLoc), baseLoc, ""))
            {
                // Try and grab a Vector 2 that isn't null and use it.
                VECTOR_2D flatDropPoint = AIPGlobalUtils::Get2DVectorFromPath(baseLoc, 0);
                
                if (!AIPGlobalUtils::IsNull2DVector(flatDropPoint))
                {
                    findSpot = false;
                    dropPoint.x = flatDropPoint.x;
                    dropPoint.z = flatDropPoint.z;
                }
            }
            
            if (findSpot)
            {
                // Grab the ODF for Exus2.
                char recyclerOdf[ODF_MAX_LEN] = {};
                if (GetObjInfo(recycler, Get_ODF, recyclerOdf))
                {
                    FindBuildSpot(team, "CLASS_RECYCLER", GetPosition(recycler), dropPoint, dropDir);   
                }
                else
                {
                    exu2::PrintConsoleMessage("ERROR: Unable to deploy Recycler. Could not find valid Recycler ODF. Modder, please fix.");
                }
            }
            
            char buildingDir = 0;
            if (fabsf(dropPoint.z) > fabsf(dropPoint.x))
            {
                if (dropPoint.z > 0.0f)
                {
                    buildingDir = 0;
                }
                else
                {
                    buildingDir = 2;
                }
            }
            else
            {
                if (dropPoint.x > 0.0f)
                {
                    buildingDir = 1;
                }
                else
                {
                    buildingDir = 3;
                }
            }
            
            int AIPDir = -1;
            GetODFInt(cfg, "Start", "BaseDir", &AIPDir, AIPDir);
            
            if (AIPDir >= 0 && AIPDir < 4)
            {
                buildingDir = AIPDir;
            }
            
            thisTeam->SetBuildingDir(buildingDir);
            SetCommand(recycler, CMD_DEPLOY, 0, 0, dropPoint);
        }
    }
    
    // Clean up.
    CloseODF(cfg);
}

void AIPSchedPlan::ReleaseUnits(int team, int priority)
{
}

void AIPSchedPlan::ReleaseFinishedUnits(int team, int priority)
{
}
