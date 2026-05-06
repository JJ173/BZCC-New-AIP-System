#include "TeamOverwatch.h"

#include <ExtraUtils.h>
#include "AIPGlobals.h"
#include "AIPBuildPlans.h"

TeamOverwatch AIPGlobals::TeamOverwatchState[MAX_TEAMS];

// ==================================================
// Constructor / Destructor
// ==================================================

TeamOverwatch::TeamOverwatch()
{
    GameTPS = SecondsToTurns(1.0f);
    Clean();
    
    ScavIdleTime = 15;
    AttackerIdleTime = 120;
    BomberIdleTime = 120;
    MinAttackForce = 3;
    MaxTargets = 4;
    MaxBomberTargets = 8;
}

TeamOverwatch::~TeamOverwatch()
{
    // Nothing to do for now.
}

// ==================================================
// Setup
// ==================================================

void TeamOverwatch::Setup(const int team)
{
    // Newly constructed AIP Teams will need to snag all units on the map that belong to the team.
    const bool needToAddAll = MyTeam < 0;
    MyTeam = team;

    // Expensive calls here so we need to be careful. This may cause lag. Will need to test and potentially request 
    // a method in Exus to avoid doing this.
    if (!needToAddAll)
    {
        return;
    }

    size_t bufferSize = 0;
    Handle* handleArray = nullptr;
    GetAllGameObjectHandles(bufferSize, handleArray); // Know this will fail, just fill in bufSize please
    handleArray = new Handle[bufferSize];

    if (GetAllGameObjectHandles(bufferSize, handleArray))
    {
        for (size_t i = 0; i < bufferSize; ++i)
        {
            const Handle handle = handleArray[i];

            if (!IsCraftButNotPerson(handle))
            {
                continue;
            }

            if (GetTeamNum(handle) != MyTeam)
            {
                continue;
            }
                        
            AddVehicle(handle);
        }
    }

    delete[] handleArray;
}

void TeamOverwatch::LoadTuning(std::string AIPFile)
{
    TeamAIPFile = AIPFile;
    const char* teamAIPFileCString = AIPFile.c_str();

    int teamScrap = GetScrap(MyTeam);
    int scrapCount = 0;

    // Load Start Values.
    GetODFBool(teamAIPFileCString, AIPGlobals::StartString, "SpawnRecycler", &SpawnRecycler, false);
    GetODFBool(teamAIPFileCString, AIPGlobals::StartString, "CheckProvides", &CheckProvides, true);
    GetODFBool(teamAIPFileCString, AIPGlobals::StartString, "DLLCanSwitch", &DLLCanSwitch, false);
    GetODFBool(teamAIPFileCString, AIPGlobals::StartString, "IdleAllCraft", &IdleAllCraft, false);
    GetODFString(teamAIPFileCString, AIPGlobals::StartString, "ScavClass", sizeof(ScavClass), ScavClass, "");
    GetODFString(teamAIPFileCString, AIPGlobals::StartString, "ConsClass", sizeof(ConsClass), ConsClass, "");
    GetODFInt(teamAIPFileCString, AIPGlobals::StartString, "ScrapCount", &scrapCount, teamScrap);

    if (SpawnRecycler)
    {
        GetODFString(teamAIPFileCString, AIPGlobals::StartString, "SpawnPath", sizeof(SpawnPath), SpawnPath, "");
        GetODFString(teamAIPFileCString, AIPGlobals::StartString, "RecyclerODF", sizeof(RecyclerODF), RecyclerODF, "");

        if (!DoesODFExist(RecyclerODF))
        {
            exu2::PrintConsoleMessage("AIP ERROR: {} ODF does not exist.", RecyclerODF);
        }
        else
        {
            PreloadODF(RecyclerODF);
            Recycler = BuildObject(RecyclerODF, MyTeam, GetPosition(SpawnPath, 0));
        }
    }

    if (ScavClass[0] = '\0')
    {
        exu2::PrintConsoleMessage("AIP ERROR: {} [Start]::scavClass not set.", AIPFile);
    }

    if (ConsClass[0] = '\0')
    {
        exu2::PrintConsoleMessage("AIP ERROR: {} [Start]::consClass not set.", AIPFile);
    }

    if (IdleAllCraft)
    {
        for (auto& vehicle : TrackedVehicles)
        {
            if (!OpenODF(vehicle.ODF))
            {
                return;
            }

            bool doIdleDispatch = true;
            GetODFBool(vehicle.ODF, "CraftClass", "DoIdleDispatch", &doIdleDispatch, true);

            if (!doIdleDispatch)
            {
                CloseODF(vehicle.ODF);
                continue;
            }

            Stop(vehicle.Handle, 0);
            CloseODF(vehicle.ODF);
        }
    }

    if (teamScrap < scrapCount)
    {
        SetScrap(MyTeam, scrapCount);
    }

    // Load Idle Dispatcher Values.
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "ScavIdleSeconds", &ScavIdleTime, 15);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "UnitIdleSeconds", &AttackerIdleTime, 120);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "BomberIdleSeconds", &BomberIdleTime, 120);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "MinAttackForce", &MinAttackForce, 3);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "MaxAttackForce", &MaxAttackForce, 99);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "MinMissionTime", &MinMissionTime, 60 * 6);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "MaxTargets", &MaxTargets, 4);
    GetODFInt(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "MaxBomberTargets", &MaxBomberTargets, 8);
    GetODFBool(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "CraftTeamIsPilotTeam", &CraftTeamIsPilotTeam, false);
    GetODFBool(teamAIPFileCString, AIPGlobals::IdleDispatcherString, "SendAllUnitsOnStartup", &SendAllIdleOnStartup, false);

    // Load Cheat Values.
    GetODFInt(teamAIPFileCString, AIPGlobals::CheatString, "MoneyAmount", &MoneyAmount, 0);
    GetODFFloat(teamAIPFileCString, AIPGlobals::CheatString, "MoneyDelay", &MoneyDelay, 0);

    if (MoneyAmount > 0)
    {
        MoneyTime = AIPGlobals::CurrentGameTurn + SecondsToTurns(MoneyDelay);
    }

    // Convert seconds to ticks.
    ScavIdleTime *= GameTPS;
    AttackerIdleTime *= GameTPS;

    if (SendAllIdleOnStartup)
    {
        SendAllIdle();
    }
}

void TeamOverwatch::FirstLoad()
{
    // Check what "First Load" is from the SaveType. Will need to talk to VT again about `ILoadSaveVisitor`.
    // bool bFirstLoad = (ILoadSaveVisitor::GetCurrentSaveType() != ST_SAVE) && (ILoadSaveVisitor::GetCurrentSaveType() != ST_JOIN);
    bool firstLoad = true;

    if (AIPGlobals::SchedPlan[MyTeam][0] == nullptr && firstLoad)
    {
        Handle recycler = GetRecycler();

        // TODO: Ensure this team isn't a player team, otherwise we can take direct control of the Recycler.
        if (IsAlive(recycler) && !IsBuilding(recycler))
        {
            const char* teamAIPFileCString = TeamAIPFile.c_str();
            Vector dropPoint{ 0.0f, 0.0f, 0.0f }, dropDir{ 0.0f, 0.0f, 0.0f };

            bool findSpot = true;
            char baseLoc[ODF_MAX_LEN];

            if (GetODFString(teamAIPFileCString, AIPGlobals::StartString, "BaseLoc", sizeof(baseLoc), baseLoc, ""))
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
                char recyclerOdf[ODF_MAX_LEN] = {};
                if (GetObjInfo(recycler, Get_ODF, recyclerOdf))
                {
                    FindBuildSpot(MyTeam, recyclerOdf, GetPosition(recycler), dropPoint, dropDir);
                }
                else
                {
                    exu2::PrintConsoleMessage("AIP ERROR: Unable to deploy Recycler. Could not find valid Recycler ODF. Modder, please fix.");
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
            GetODFInt(teamAIPFileCString, AIPGlobals::StartString, "BaseDir", &AIPDir, AIPDir);

            if (AIPDir >= 0 && AIPDir < 4)
            {
                buildingDir = AIPDir;
            }

            SetBuildingDir(buildingDir);
            SetCommand(recycler, CMD_DEPLOY, 0, 0, dropPoint);
        }
    }
}

// TODO: Return to this.
void TeamOverwatch::LoadMatches()
{

}

void TeamOverwatch::LoadPlanNames()
{
    PlanNames.clear();
    for (int i = 1; i <= AIPGlobals::MAX_PLANS; ++i)
    {
        PlanNames.push_back(std::format("Plan{}", i));
    }
}

void TeamOverwatch::LoadScheduledPlans()
{
    const char* teamAIPFileCString = TeamAIPFile.c_str();
    int slot = 0;
    for (const auto& sectionStr : PlanNames)
    {
        if (slot >= AIPGlobals::MAX_PLANS)
        {
            break;
        }

        const char* section = sectionStr.c_str();
        char planType[ODF_MAX_LEN] = "";

        if (!GetODFString(teamAIPFileCString, section, "planType", sizeof(planType), planType, planType))
        {
            break;
        }

        AIPSchedPlan* plan;

        if (!_stricmp(planType, "BuildMinimums")) 
        {
            plan = new BuildMinimums(MyTeam);
        }
        else
        {
            plan = nullptr;
        }

        if (plan == nullptr)
        {
            continue;
        }

        GetODFInt(teamAIPFileCString, section, "planPriority", &plan->Priority);
        GetODFBool(teamAIPFileCString, section, "ContinueEvenIfFail", &plan->ContinueEvenIfFail, false);

        GetODFInt(teamAIPFileCString, section, "MinTtlScrap", &plan->MinTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "MaxTtlScrap", &plan->MaxTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "MinCurScrap", &plan->MinCurScrap, 0);
        GetODFInt(teamAIPFileCString, section, "MaxCurScrap", &plan->MaxCurScrap, 0);

        AIPGlobalUtils::Clamp(plan->MinTtlScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->MaxTtlScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->MinCurScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->MaxCurScrap, 0, 99999);

        GetODFInt(teamAIPFileCString, section, "HisMinTtlScrap", &plan->HisMinTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMaxTtlScrap", &plan->HisMaxTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMinCurScrap", &plan->HisMinCurScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMaxCurScrap", &plan->HisMaxCurScrap, 0);

        AIPGlobalUtils::Clamp(plan->HisMinTtlScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->HisMaxTtlScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->HisMinCurScrap, 0, 99999);
        AIPGlobalUtils::Clamp(plan->HisMaxCurScrap, 0, 99999);

        GetODFInt(teamAIPFileCString, section, "MinTtlPower", &plan->MinTtlPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MaxTtlPower", &plan->MaxTtlPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MinCurPower", &plan->MinCurPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MaxCurPower", &plan->MaxCurPower, AIPGlobals::IGNORE_POWER);

        AIPGlobalUtils::Clamp(plan->MinTtlPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->MaxTtlPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->MinCurPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->MaxCurPower, AIPGlobals::IGNORE_POWER, 99999);

        GetODFInt(teamAIPFileCString, section, "HisMinTtlPower", &plan->HisMinTtlPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMaxTtlPower", &plan->HisMaxTtlPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMinCurPower", &plan->HisMinCurPower, AIPGlobals::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMaxCurPower", &plan->HisMaxCurPower, AIPGlobals::IGNORE_POWER);

        AIPGlobalUtils::Clamp(plan->HisMinTtlPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->HisMaxTtlPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->HisMinCurPower, AIPGlobals::IGNORE_POWER, 99999);
        AIPGlobalUtils::Clamp(plan->HisMaxCurPower, AIPGlobals::IGNORE_POWER, 99999);

        // Look for conditions.
        char planIf[ODF_MAX_LEN] = {};
        int planIfCount;
        const char* planIfClass;

        for (int i = 0; i < AIPGlobals::MAX_PLAN_CONDITIONS; i++)
        {

        }

        plan->Init(teamAIPFileCString, section);
        Plans.push_back(plan);
    }
}

void TeamOverwatch::ClearPlans()
{
    // TODO: See if these are used anywhere outside of this class, else we can make them local.
    AIPGlobals::ConstructorReserveScrap[MyTeam] = 0;
    AIPGlobals::ConstructorReservePriority[MyTeam] = 0;
    AIPGlobals::ConstructorReserveTime[MyTeam] = 0;

    for (auto& plan : Plans)
    {
        plan->Done();
    }

    Plans.clear();

    // TODO: remove this check and ensure the teams are set before this is called.
    if (MyTeam > 0) 
    {
        AIPGlobals::LastAIPFile[MyTeam] = 0;
    }
}

// ==================================================
// Events
// ==================================================

void TeamOverwatch::AddVehicle(const Handle handle)
{
    if (MyTeam < 0)
    {
        return;
    }

    if (NumVehicles >= AIPGlobals::MAX_UNITS_PER_TEAM)
    {
        return;
    }

    if (!IsCraftButNotPerson(handle))
    {
        return;
    }
    
    char objClass[ODF_MAX_LEN] = {};
    if (!GetObjInfo(handle, Get_GOClass, objClass))
    {
        return;
    }
    
    if (AIPGlobalUtils::IsRecycler(objClass))
    {
        Recycler = handle;
        return; 
    }
    
    for (const char* excludedClass : ExcludedVehicleClasses)
    {
        if (strcmp(objClass, excludedClass) == 0)
        {            
            return;
        }       
    }
    
    for (const auto& vehicle : TrackedVehicles)
    {
        if (vehicle.Handle == handle)
        {
            return;
        }       
    }
    
    bool doIdleDispatch = true;
    char objOdf[ODF_MAX_LEN] = {};
    if (!GetObjInfo(handle, Get_ODF, objOdf))
    {
        return;
    }

    if (!OpenODF(objOdf))
    {
        return;
    }

    GetODFBool(objOdf, "CraftClass", "DoIdleDispatch", &doIdleDispatch, true);

    if (!doIdleDispatch)
    {
        CloseODF(objOdf);
        return;
    }
        
    AIPGlobals::GameObjectInfo newVehicle;
    newVehicle.Handle = handle;
    newVehicle.LastPosition = GetPosition(handle);
    newVehicle.IdleTime = 0;
    newVehicle.Class = objClass;
    newVehicle.ODF = objOdf;
    
    if (strcmp(objClass, "CLASS_SCAVENGER") == 0 || strcmp(objClass, "CLASS_SCAVENGERH") == 0)
    {
        newVehicle.Type = AIPGlobals::SCAV;
    }
    else if (strcmp(objClass, "CLASS_TURRETTANK") == 0)
    {
        newVehicle.Type = AIPGlobals::TURRET;
    }
    else if (strcmp(objClass, "CLASS_CONSTRUCTIONRIG") == 0 || strcmp(objClass, "CLASS_CONSTRUCTIONRIGT") == 0)
    {
        newVehicle.Type = AIPGlobals::CONSTRUCTOR;
    }
    else if (strcmp(objClass, "CLASS_SERVICE") == 0 || strcmp(objClass, "CLASS_SERVICEH") == 0)
    {
        newVehicle.Type = AIPGlobals::SERVICE_TRUCK;
    }
    else if (strcmp(objClass, "CLASS_BOMBER") == 0)
    {
        newVehicle.Type = AIPGlobals::BOMBER;
    }
    else
    {
        newVehicle.Type = AIPGlobals::GENERAL;
    }
    
    TrackedVehicles.push_back(newVehicle);
    NumVehicles++;
    CloseODF(objOdf);
}

void TeamOverwatch::DeleteVehicle(const Handle handle)
{
    if (MyTeam < 0)
    {
        return;
    }

    for (int i = 0; i < NumVehicles; ++i)
    {
        const AIPGlobals::GameObjectInfo& vehicle = TrackedVehicles[i];
        if (vehicle.Handle == handle)
        {
            TrackedVehicles.erase(TrackedVehicles.begin() + i);
            NumVehicles--;
            break;
        }
    }
}

void TeamOverwatch::Execute()
{
    if (MyTeam < 0)
    {
        return;
    }

    CullVehicles();

    // Temp.
    if (!Plans.empty())
    {
        for (auto& plan : Plans)
        {
            plan->Execute();
        }
    }
}

// ==================================================
// Utilities
// ==================================================

void TeamOverwatch::Clean()
{
    MyTeam = -1;
    TrackedVehicles = std::vector<AIPGlobals::GameObjectInfo>();
    NumVehicles = 0;
}

bool TeamOverwatch::FindBuildSpot(int team, const char* objOdf, const Vector& start, Vector& pos, Vector& dir)
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
    dir = Vector{ 0.0f, 0.0f, 1.0f };
    return false;
}

void TeamOverwatch::CullVehicles()
{
    bool anyChanges = false;

    do
    {
        anyChanges = false;

        for (int i = 0; i < NumVehicles; ++i)
        {
            AIPGlobals::GameObjectInfo& vehicle = TrackedVehicles[i];

            if (!IsAlive(vehicle.Handle))
            {
                DeleteVehicle(vehicle.Handle);
                anyChanges = true;
                break;
            }

            if (GetTeamNum(vehicle.Handle) != MyTeam)
            {
                DeleteVehicle(vehicle.Handle);
                anyChanges = true;
                break;
            }
        }
    } while (anyChanges);
}

void TeamOverwatch::SendAllIdle()
{
    if (MyTeam < 0)
    {
        return;
    }

    // Make sure the list is valid before we proceed.
    CullVehicles();

    for (auto& vehicle : TrackedVehicles)
    {
        if (vehicle.Type != AIPGlobals::GENERAL && vehicle.Type != AIPGlobals::BOMBER)
        {
            continue;
        }

        const Vector& currentPosition = GetPosition(vehicle.Handle);

    }
}

int TeamOverwatch::CountIdleScavs()
{
    if (MyTeam < 0)
    {
        return 0;
    }
    
    int idleCounter = 0;
    
    for (int i = 0; i < NumVehicles; ++i)
    {
        const AIPGlobals::GameObjectInfo& vehicle = TrackedVehicles[i];
        if (vehicle.Type == AIPGlobals::SCAV && vehicle.IdleTime > ScavIdleTime)
        {
            ++idleCounter;
        }
    }
    
    return idleCounter;
}

// ==================================================
// Getters/Setters
// ==================================================

int TeamOverwatch::GetBuildingDir()
{
    return BuildingDirection;
}

void TeamOverwatch::SetBuildingDir(char buildingDir)
{
    BuildingDirection = buildingDir;
}

Handle TeamOverwatch::GetRecycler() const
{
    if (Recycler)
    {
        return Recycler;
    }
    
    return GetObjectByTeamSlot(MyTeam, DLL_TEAM_SLOT_RECYCLER);
}