#pragma once

#include <ScriptUtils.h>
#include <vector>

#include "AIPGlobals.h"

class TeamOverwatch
{
    Handle Recycler;
    std::vector<AIPGlobals::GameObjectInfo> TrackedVehicles;
    
    static constexpr const char* ExcludedVehicleClasses[] =
    {
        "CLASS_TURRET",
        "CLASS_TUG"
    };
    
    int GameTPS;
    int NumVehicles;
    int MyTeam;
    
    int BuildingDirection;
    
    int ScavIdleTime;
    int AttackerIdleTime;
    int BomberIdleTime;
    int MinAttackForce;
    int MaxAttackForce;
    int MinMissionTime;
    int MaxTargets;
    int MaxBomberTargets;
    
    bool IdleAllCraft;
    bool CheckProvides;
    bool DLLCanSwitch;
    
    bool CraftTeamIsPilotTeam;
    bool SendAllIdleOnStartup;
    
public:
    TeamOverwatch();
    ~TeamOverwatch();
    
    void Clean();
    void LoadTuning(const char* AIPFile);
    void Setup(int Team);
    void AddVehicle(Handle handle);
    void DeleteVehicle(Handle handle);
    void CullVehicles();
    void SendAllIdle();
    
    void Execute();
    void Save(bool missionSave);
    void Load(bool missionSave);
    void PostLoad();
    
    int CountIdleScavs();
    Handle GetRecycler() const;
    
    int GetBuildingDir();
    void SetBuildingDir(char buildingDir);
};
