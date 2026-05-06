#pragma once

#include <ScriptUtils.h>
#include <vector>
#include <string>
#include <string_view>
#include <ranges>

#include "AIPSchedPlan.h"
#include "AIPGlobals.h"

class TeamOverwatch
{
    Handle Recycler;

    std::string TeamAIPFile;

    std::vector<AIPGlobals::GameObjectInfo> TrackedVehicles;
    std::vector<AIPGlobals::MatchTargetInfo> MatchTargets;
    std::vector<std::string> PlanNames;
    std::vector<AIPSchedPlan*> Plans;
    
    static constexpr const char* ExcludedVehicleClasses[] =
    {
        "CLASS_TURRET",
        "CLASS_TUG"
    };
    
    char SpawnPath[ODF_MAX_LEN];
    char RecyclerODF[ODF_MAX_LEN];

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
    int MoneyAmount;
    float MoneyDelay;
    int MoneyTime;
    
    bool SpawnRecycler;
    bool IdleAllCraft;
    bool CheckProvides;
    bool DLLCanSwitch;
    
    bool CraftTeamIsPilotTeam;
    bool SendAllIdleOnStartup;

    bool FindBuildSpot(int team, const char* objOdf, const Vector& start, Vector& pos, Vector& dir);
    
public:
    TeamOverwatch();
    ~TeamOverwatch();
    
    void Clean();

    void LoadTuning(std::string AIPFile);
    void Setup(int Team);
    void FirstLoad();
    void LoadMatches();
    void LoadPlanNames();
    void LoadScheduledPlans();
    void ClearPlans();

    void AddVehicle(Handle handle);
    void DeleteVehicle(Handle handle);    
    void Execute();
    void Save(bool missionSave);
    void Load(bool missionSave);
    void PostLoad();
    
    void CullVehicles();
    void SendAllIdle();

    int CountIdleScavs();
    Handle GetRecycler() const;
    
    int GetBuildingDir();
    void SetBuildingDir(char buildingDir);

    char ScavClass[ODF_MAX_LEN] = {};
    char ConsClass[ODF_MAX_LEN] = {};
};