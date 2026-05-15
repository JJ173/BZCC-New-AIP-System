#include "TeamOverwatch.hpp"
#include "AIPSchedPlan.hpp"
#include "BuilderPlans.hpp"
#include "CollectorPlans.hpp"

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
    GetODFBool(teamAIPFileCString, Constants::StartString, "SpawnRecycler", &SpawnRecycler, false);
    GetODFBool(teamAIPFileCString, Constants::StartString, "CheckProvides", &CheckProvides, true);
    GetODFBool(teamAIPFileCString, Constants::StartString, "DLLCanSwitch", &DLLCanSwitch, false);
    GetODFBool(teamAIPFileCString, Constants::StartString, "IdleAllCraft", &IdleAllCraft, false);
    GetODFString(teamAIPFileCString, Constants::StartString, "ScavClass", sizeof(ScavClass), ScavClass, "");
    GetODFString(teamAIPFileCString, Constants::StartString, "ConsClass", sizeof(ConsClass), ConsClass, "");
    GetODFInt(teamAIPFileCString, Constants::StartString, "ScrapCount", &scrapCount, teamScrap);

    if (SpawnRecycler)
    {
        GetODFString(teamAIPFileCString, Constants::StartString, "SpawnPath", sizeof(SpawnPath), SpawnPath, "");
        GetODFString(teamAIPFileCString, Constants::StartString, "RecyclerODF", sizeof(RecyclerODF), RecyclerODF, "");

        if (!DoesODFExist(RecyclerODF))
        {
            exu2::PrintConsoleMessage("AIP ERROR: {} ODF does not exist.", RecyclerODF);
        }
        else
        {
            PreloadODF(RecyclerODF);
            BuildObject(RecyclerODF, MyTeam, GetPosition(SpawnPath, 0));
        }
    }

    if (ScavClass == nullptr)
    {
        exu2::PrintConsoleMessage("AIP ERROR: {} [Start]::scavClass not set.", AIPFile);
    }

    if (ConsClass == nullptr)
    {
        exu2::PrintConsoleMessage("AIP ERROR: {} [Start]::consClass not set.", AIPFile);
    }

    if (IdleAllCraft)
    {
        for (auto& vehicle : TrackedVehicles)
        {
            if (!OpenODF(vehicle->ODF.c_str()))
            {
                return;
            }

            bool doIdleDispatch = true;
            GetODFBool(vehicle->ODF.c_str(), "CraftClass", "DoIdleDispatch", &doIdleDispatch, true);

            if (!doIdleDispatch)
            {
                CloseODF(vehicle->ODF.c_str());
                continue;
            }

            Stop(vehicle->Handle, 0);
            CloseODF(vehicle->ODF.c_str());
        }
    }

    if (teamScrap < scrapCount)
    {
        SetScrap(MyTeam, scrapCount);
    }

    // Load Idle Dispatcher Values.
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "ScavIdleSeconds", &ScavIdleTime, 15);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "UnitIdleSeconds", &AttackerIdleTime, 120);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "BomberIdleSeconds", &BomberIdleTime, 120);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "MinAttackForce", &MinAttackForce, 3);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "MaxAttackForce", &MaxAttackForce, 99);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "MinMissionTime", &MinMissionTime, 60 * 6);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "MaxTargets", &MaxTargets, 4);
    GetODFInt(teamAIPFileCString, Constants::IdleDispatcherString, "MaxBomberTargets", &MaxBomberTargets, 8);
    GetODFBool(teamAIPFileCString, Constants::IdleDispatcherString, "CraftTeamIsPilotTeam", &CraftTeamIsPilotTeam, false);
    GetODFBool(teamAIPFileCString, Constants::IdleDispatcherString, "SendAllUnitsOnStartup", &SendAllIdleOnStartup, false);

    // Load Cheat Values.
    GetODFInt(teamAIPFileCString, Constants::CheatString, "MoneyAmount", &MoneyAmount, 0);
    GetODFFloat(teamAIPFileCString, Constants::CheatString, "MoneyDelay", &MoneyDelay, 0);

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

    if (Plans.empty() && firstLoad)
    {
        Handle recycler = Recycler.ProdHandle;

        // TODO: Ensure this team isn't a player team, otherwise we can take direct control of the Recycler.
        if (IsAlive(recycler) && !IsBuilding(recycler))
        {
            const char* teamAIPFileCString = TeamAIPFile.c_str();
            Vector dropPoint{ 0.0f, 0.0f, 0.0f }, dropDir{ 0.0f, 0.0f, 0.0f };

            bool findSpot = true;
            char baseLoc[ODF_MAX_LEN];

            if (GetODFString(teamAIPFileCString, Constants::StartString, "BaseLoc", sizeof(baseLoc), baseLoc, ""))
            {
                // Try and grab a Vector 2 that isn't null and use it.
                VECTOR_2D flatDropPoint = Utilities::Get2DVectorFromPath(baseLoc, 0);

                if (!Utilities::IsNull2DVector(flatDropPoint))
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
            GetODFInt(teamAIPFileCString, Constants::StartString, "BaseDir", &AIPDir, AIPDir);

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
    for (int i = 1; i <= Constants::MAX_PLANS; ++i)
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
        if (slot >= Constants::MAX_PLANS)
        {
            break;
        }

        const char* section = sectionStr.c_str();
        char planType[ODF_MAX_LEN] = "";

        if (!GetODFString(teamAIPFileCString, section, "planType", sizeof(planType), planType, planType))
        {
            break;
        }

        std::unique_ptr<AIPSchedPlan> plan;

        if (!_stricmp(planType, "BuildMinimums")) 
        {
            plan = std::make_unique<BuildMinimums>(MyTeam, &AIPGlobals::TeamOverwatchState[MyTeam]);
        }
        else if (!_stricmp(planType, "CollectPool"))
        {
            plan = std::make_unique<CollectPool>(MyTeam, &AIPGlobals::TeamOverwatchState[MyTeam]);
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

        Utilities::Clamp(plan->MinTtlScrap, 0, 99999);
        Utilities::Clamp(plan->MaxTtlScrap, 0, 99999);
        Utilities::Clamp(plan->MinCurScrap, 0, 99999);
        Utilities::Clamp(plan->MaxCurScrap, 0, 99999);

        GetODFInt(teamAIPFileCString, section, "HisMinTtlScrap", &plan->HisMinTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMaxTtlScrap", &plan->HisMaxTtlScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMinCurScrap", &plan->HisMinCurScrap, 0);
        GetODFInt(teamAIPFileCString, section, "HisMaxCurScrap", &plan->HisMaxCurScrap, 0);

        Utilities::Clamp(plan->HisMinTtlScrap, 0, 99999);
        Utilities::Clamp(plan->HisMaxTtlScrap, 0, 99999);
        Utilities::Clamp(plan->HisMinCurScrap, 0, 99999);
        Utilities::Clamp(plan->HisMaxCurScrap, 0, 99999);

        GetODFInt(teamAIPFileCString, section, "MinTtlPower", &plan->MinTtlPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MaxTtlPower", &plan->MaxTtlPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MinCurPower", &plan->MinCurPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "MaxCurPower", &plan->MaxCurPower, Constants::IGNORE_POWER);

        Utilities::Clamp(plan->MinTtlPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->MaxTtlPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->MinCurPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->MaxCurPower, Constants::IGNORE_POWER, 99999);

        GetODFInt(teamAIPFileCString, section, "HisMinTtlPower", &plan->HisMinTtlPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMaxTtlPower", &plan->HisMaxTtlPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMinCurPower", &plan->HisMinCurPower, Constants::IGNORE_POWER);
        GetODFInt(teamAIPFileCString, section, "HisMaxCurPower", &plan->HisMaxCurPower, Constants::IGNORE_POWER);

        Utilities::Clamp(plan->HisMinTtlPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->HisMaxTtlPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->HisMinCurPower, Constants::IGNORE_POWER, 99999);
        Utilities::Clamp(plan->HisMaxCurPower, Constants::IGNORE_POWER, 99999);

        // Look for conditions.
        char planIf[ODF_MAX_LEN] = {};
        int planIfCount;
        const char* planIfClass;

        for (int i = 0; i < Constants::MAX_PLAN_CONDITIONS; i++)
        {

        }

        plan->Init(teamAIPFileCString, section);
        
        Plans.push_back(std::move(plan));
        slot++;
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

void TeamOverwatch::AddProducer(const Handle handle, std::string objClass, std::string objOdf)
{
    if (GetTeamNum(handle) != MyTeam)
    {
        return;
    }

    Producer newProd = {};
    newProd.ProdHandle = handle;
    newProd.ObjClass = objClass;
    newProd.ObjODF = objOdf;

    if (Utilities::IsRecycler(objClass.c_str()))
    {
        Recycler = newProd;
    }
    else
    {
        Producers.push_back(std::move(std::make_unique<Producer>(newProd)));
    }
}

void TeamOverwatch::AddVehicle(const Handle handle)
{
    if (MyTeam < 0)
    {
        return;
    }

    if (NumVehicles >= Constants::MAX_UNITS_PER_TEAM)
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
        
    for (const char* excludedClass : ExcludedVehicleClasses)
    {
        if (strcmp(objClass, excludedClass) == 0)
        {            
            return;
        }       
    }
    
    for (const auto& vehicle : TrackedVehicles)
    {
        if (vehicle->Handle == handle)
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
        
    Vehicle newVehicle{};
    newVehicle.Handle = handle;
    newVehicle.LastPosition = GetPosition(handle);
    newVehicle.IdleTime = 0;
    newVehicle.Class = objClass;
    newVehicle.ODF = objOdf;
    
    if (strcmp(objClass, "CLASS_SCAVENGER") == 0 || strcmp(objClass, "CLASS_SCAVENGERH") == 0)
    {
        newVehicle.Type = Enums::SCAV;
    }
    else if (strcmp(objClass, "CLASS_TURRETTANK") == 0)
    {
        newVehicle.Type = Enums::TURRET;
    }
    else if (strcmp(objClass, "CLASS_CONSTRUCTIONRIG") == 0 || strcmp(objClass, "CLASS_CONSTRUCTIONRIGT") == 0)
    {
        newVehicle.Type = Enums::CONSTRUCTOR;
    }
    else if (strcmp(objClass, "CLASS_SERVICE") == 0 || strcmp(objClass, "CLASS_SERVICEH") == 0)
    {
        newVehicle.Type = Enums::SERVICE_TRUCK;
    }
    else if (strcmp(objClass, "CLASS_BOMBER") == 0)
    {
        newVehicle.Type = Enums::BOMBER;
    }
    else
    {
        newVehicle.Type = Enums::GENERAL;
    }
    
    TrackedVehicles.push_back(std::move(std::make_unique<Vehicle>(newVehicle)));
    NumVehicles++;
    CloseODF(objOdf);
}

void TeamOverwatch::DeleteVehicle(const Handle handle)
{
    if (MyTeam < 0)
    {
        return;
    }

    for (size_t i = 0; i < NumVehicles; ++i)
    {
        std::unique_ptr<Vehicle>& vehicle = TrackedVehicles[i];
        if (vehicle->Handle == handle)
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
}

// ==================================================
// Utilities
// ==================================================

void TeamOverwatch::Clean()
{
    MyTeam = -1;
    TrackedVehicles.clear();
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
    std::sort(&points[0], &points[99], Utilities::ComparePoints);
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

        for (size_t i = 0; i < NumVehicles; ++i)
        {
            std::unique_ptr<Vehicle>& vehicle = TrackedVehicles[i];

            if (!IsAlive(vehicle->Handle))
            {
                DeleteVehicle(vehicle->Handle);
                anyChanges = true;
                break;
            }

            if (GetTeamNum(vehicle->Handle) != MyTeam)
            {
                DeleteVehicle(vehicle->Handle);
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
        if (vehicle->Type != Enums::GENERAL && vehicle->Type != Enums::BOMBER)
        {
            continue;
        }

        const Vector& currentPosition = GetPosition(vehicle->Handle);

    }
}

int TeamOverwatch::CountIdleScavs(const char* scavClass)
{
    if (MyTeam < 0)
    {
        return 0;
    }
    
    int idleCounter = 0;
    
    for (size_t i = 0; i < NumVehicles; ++i)
    {
        std::unique_ptr<Vehicle>& vehicle = TrackedVehicles[i];
        if (vehicle->Type == Enums::SCAV && vehicle->IdleTime > ScavIdleTime)
        {
            if (scavClass == nullptr || vehicle->Class == scavClass)
            {
                ++idleCounter;
            }
        }
    }
    
    return idleCounter;
}

Handle TeamOverwatch::ActivateIdleScavNear(const Vector& v, const char* scavClass)
{
    if (MyTeam < 0)
    {
        return 0;
    }

    CullVehicles();

    int bestMatch = -1;
    float minDist = Constants::MAX_FLOAT;

    for (size_t i = 0; i < NumVehicles; i++)
    {
        std::unique_ptr<Vehicle>& vehicle = TrackedVehicles[i];

        if (vehicle->Type != Enums::SCAV || vehicle->IdleTime < ScavIdleTime)
        {
            continue;
        }

        if (scavClass == nullptr || vehicle->Class == scavClass)
        {
            float curDist = (v.x - vehicle->LastPosition.x) * (v.x - vehicle->LastPosition.x) + (v.z - vehicle->LastPosition.z) * (v.z - vehicle->LastPosition.z);

            if (curDist < minDist)
            {
                minDist = curDist;
                bestMatch = i;
            }
        }
    }

    if (bestMatch < 0)
    {
        return 0;
    }

    std::unique_ptr<Vehicle>& bestVehicle= TrackedVehicles[bestMatch];
    bestVehicle->IdleTime = -100;
    return bestVehicle->Handle;
}

// ==================================================
// Build Methods
// ==================================================

bool TeamOverwatch::UpdateBuild(std::unique_ptr<BuildRequest>& request, bool checkProvides)
{
    bool doMore = true;

    switch (request->Stage)
    {
        case Enums::BUILD_TRY:
        {
            Enums::TTB_RESULT result = TryToBuild(request);

            if (result == Enums::BUILDING)
            {
                request->Stage = Enums::BUILD_COMMIT;
                return !doMore;
            }

            break;
        }
        case Enums::BUILD_COMMIT:
        {
            // Start the build process.
            CommitBuild(request);
            request->StartTime = AIPGlobals::CurrentGameTurn;
            request->Stage = Enums::BUILD_WAIT;
            return !doMore;
            break;
        }
        case Enums::BUILD_WAIT:
        {
            if (WaitBuild(request, checkProvides))
            {
                request->Stage = Enums::BUILD_DONE;
            }

            break;
        }
        case Enums::BUILD_DONE:
        {
            break;
        }
    }

    return false;
}

void TeamOverwatch::UpdateBuilds(bool checkProvides)
{
    for (auto& req : BuildRequests)
    {
        bool doMore = UpdateBuild(req, checkProvides);

        if (!doMore)
        {
            break;
        }
    }
}

Enums::TTB_RESULT TeamOverwatch::TryToBuild(std::unique_ptr<BuildRequest>& request)
{
    if (request == nullptr)
    {
        return Enums::TTB_RESULT::NOPRODUCER;
    }

    if (request->Team < 0 || request->Team >= MAX_TEAMS)
    {
        return Enums::TTB_RESULT::BADTEAM;
    }

    if (request->ObjOdf.empty())
    {
        return Enums::TTB_RESULT::NOPRODUCER;
    }

    if (!IsAround(request->AssignedProducer->ProdHandle))
    {
        return Enums::TTB_RESULT::NOPRODUCER;
    }

    if (request->AssignedProducer->IsProducerBusy())
    {
        return Enums::TTB_RESULT::PRODUCERBUSY;
    }

    // Check how much scrap this is going to cost, versus how much we have.
    long scrap = request->Cost;
    long teamScrap = GetScrap(request->Team);
    long maxScrap = GetMaxScrap(request->Team);

    // Costs way too much, we are poor.
    if (scrap > maxScrap)
    {
        return Enums::TTB_RESULT::OVERMAXSCRAP;
    }

    if (scrap > teamScrap)
    {
        return Enums::TTB_RESULT::OVERCURSCRAP;
    }

    int cReserve = AIPGlobals::ConstructorReserveScrap[request->Team];
    if (request->Priority > AIPGlobals::ConstructorReservePriority[request->Team])
    {
        cReserve = 0;
    }

    if (teamScrap < (cReserve + request->Cost))
    {
        return Enums::TTB_RESULT::OVERRESERVESCRAP;
    }

    // Check the requirements of the object here to make sure we have these units.
    // TODO: Write this method.

    request->AssignedProducer->BuildItem(request->ObjOdf);
    return Enums::TTB_RESULT::BUILDING;
}

void TeamOverwatch::CommitBuild(std::unique_ptr<BuildRequest>& request)
{
    // Make sure the Producer is still alive.
    if (request->AssignedProducer != nullptr && request->AssignedProducer->IsProducerAlive())
    {
        SetCommand(request->AssignedProducer->ProdHandle, CMD_DONE);
    }
}

bool TeamOverwatch::WaitBuild(std::unique_ptr<BuildRequest>& request, bool checkProvides)
{
    request->BuildHandle = 0;

    if (request->AssignedProducer == nullptr || !request->AssignedProducer->IsProducerAlive())
    {
        return true;
    }

    if (request->AssignedProducer->IsProducerBusy())
    {
        return false;
    }

    // Try and create a method that'll grab the "Next Command" of the producer to see if anything is queued.

    // Also need to check if this handle that's being built is not a power-up.
    AIPGlobals::HandleList emptyList;
    Handle obj = FindIdleUnit(request->ObjOdf, checkProvides, emptyList);

    if (obj != 0)
    {
        Stop(obj, 1);
        return true;
    }

    return true;
}

// ==================================================
// Finders
// ==================================================

Handle TeamOverwatch::FindIdleUnit(std::string objClass, bool checkProvides, const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle)
{
    // Iterate through our list of tracked vehicles.
    for (size_t i = 0; i < TrackedVehicles.size(); )
    {
        std::unique_ptr<Vehicle>& vehicle = TrackedVehicles[i];

        if (vehicle == nullptr)
        {
            continue;
        }

        if (GetTeamNum(vehicle->Handle) != MyTeam)
        {
            continue;
        }

        // Check provides here.

        // Ensure we're not excluded.
        AIPGlobals::HandleList::const_iterator j;
        bool dupe = false;

        for (j = excludeList.begin(); j != excludeList.end(); ++j)
        {
            if (*j == vehicle->Handle)
            {
                dupe = true;
                break;
            }
        }

        if (dupe)
        {
            continue;
        }

        return vehicle->Handle;
    }

    return 0;
}

Handle TeamOverwatch::FindIdleUnit(std::string objClass, float ammoRatio, bool checkProvides, const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle)
{
    return 0;
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

Producer* TeamOverwatch::GetRecycler()
{
    return &Recycler;
}

std::vector<std::unique_ptr<Producer>>* TeamOverwatch::GetProducers()
{
    return &Producers;
}

std::vector<std::unique_ptr<BuildRequest>>* TeamOverwatch::GetBuildRequests()
{
    return &BuildRequests;
}

std::vector<std::unique_ptr<AIPSchedPlan>>* TeamOverwatch::GetPlans()
{
    return &Plans;
}

void TeamOverwatch::AddBuildRequest(BuildRequest req)
{
    BuildRequests.push_back(std::move(std::make_unique<BuildRequest>(req)));
}

int TeamOverwatch::GetMoneyAmount() const
{
    return MoneyAmount;
}

int TeamOverwatch::GetMoneyTime() const
{
    return MoneyTime;
}

void TeamOverwatch::SetMoneyTime(int newTime)
{
    MoneyTime = newTime;
}

float TeamOverwatch::GetMoneyDelay() const
{
    return MoneyDelay;
}