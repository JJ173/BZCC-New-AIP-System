#include "TeamOverwatch.h"

#include "AIPGlobals.h"

TeamOverwatch AIPGlobals::TeamOverwatchState[MAX_TEAMS];

// ==================================================
// Team Overwatch
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

void TeamOverwatch::Clean()
{
    MyTeam = -1;
    TrackedVehicles = std::vector<AIPGlobals::GameObjectInfo>();
    NumVehicles = 0;
}

void TeamOverwatch::LoadTuning(const char* AIPFile)
{
    // Load Start Values.
    GetODFBool(AIPFile, AIPGlobals::StartString, "CheckProvides", &CheckProvides, true);
    GetODFBool(AIPFile, AIPGlobals::StartString, "DLLCanSwitch", &DLLCanSwitch, false);
    GetODFBool(AIPFile, AIPGlobals::StartString, "IdleAllCraft", &IdleAllCraft, false);
    
    // Run through our list of tracked vehicles after Setup() call, and idle them out if the ODF allows for it.
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
    
    // Load Idle Dispatcher Values.
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "ScavIdleSeconds", &ScavIdleTime, 15);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "UnitIdleSeconds", &AttackerIdleTime, 120);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "BomberIdleSeconds", &BomberIdleTime, 120);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "MinAttackForce", &MinAttackForce, 3);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "MaxAttackForce", &MaxAttackForce, 99);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "MinMissionTime", &MinMissionTime, 60 * 6);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "MaxTargets", &MaxTargets, 4);
    GetODFInt(AIPFile, AIPGlobals::IdleDispatcherString, "MaxBomberTargets", &MaxBomberTargets, 8);
    GetODFBool(AIPFile, AIPGlobals::IdleDispatcherString, "CraftTeamIsPilotTeam", &CraftTeamIsPilotTeam, false);
    GetODFBool(AIPFile, AIPGlobals::IdleDispatcherString, "SendAllUnitsOnStartup", &SendAllIdleOnStartup, false);

    // Convert seconds to ticks.
    ScavIdleTime *= GameTPS;
    AttackerIdleTime *= GameTPS;
    
    if (SendAllIdleOnStartup)
    {
        SendAllIdle();
    }
}

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
    }
    while (anyChanges);
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

void TeamOverwatch::Execute()
{
    if (MyTeam < 0)
    {
        return;
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
