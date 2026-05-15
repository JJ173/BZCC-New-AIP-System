#pragma once

#include <ExtraUtils.h>
#include <vector>
#include <string>
#include <string_view>
#include <ranges>

#include "AIPGlobals.hpp"
#include "BuildRequest.hpp"
#include "Producer.hpp"
#include "Vehicle.hpp"
#include "Utilities.hpp"

class AIPSchedPlan;

class TeamOverwatch
{
    Producer Recycler;

    std::string TeamAIPFile;

    std::vector<AIPGlobals::MatchTargetInfo> MatchTargets;
    std::vector<std::string> PlanNames;

    std::vector<std::unique_ptr<Vehicle>> TrackedVehicles;
    std::vector<std::unique_ptr<Producer>> Producers;
    std::vector<std::unique_ptr<BuildRequest>> BuildRequests;
    std::vector<std::unique_ptr<AIPSchedPlan>> Plans;
    
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
    char ScavClass[ODF_MAX_LEN];
    char ConsClass[ODF_MAX_LEN];

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

    void AddProducer(const Handle handle, std::string objClass, std::string objOdf);
    void AddVehicle(Handle handle);
    void AddBuildRequest(BuildRequest req);

    void DeleteVehicle(Handle handle);    
    void Execute();

    void Save(bool missionSave);
    void Load(bool missionSave);
    void PostLoad();
    
    void CullVehicles();
    void SendAllIdle();

    bool UpdateBuild(std::unique_ptr<BuildRequest>& request, bool checkProvides);
    void UpdateBuilds(bool checkProvides);
    Enums::TTB_RESULT TryToBuild(std::unique_ptr<BuildRequest>& request);
    void CommitBuild(std::unique_ptr<BuildRequest>& request);
    bool WaitBuild(std::unique_ptr<BuildRequest>& request, bool checkProvides);

    Handle FindIdleUnit(std::string objClass, bool checkProvides, const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);
    Handle FindIdleUnit(std::string objClass, float ammoRatio, bool checkProvides, const AIPGlobals::HandleList& excludeList, bool LookAtIsIdle = false);

    int CountIdleScavs(const char* scavClass);
    Handle ActivateIdleScavNear(const Vector& v, const char* scavClass);

    Producer* GetRecycler();

    std::vector<std::unique_ptr<Producer>>* GetProducers();
    std::vector<std::unique_ptr<BuildRequest>>* GetBuildRequests();
    std::vector<std::unique_ptr<AIPSchedPlan>>* GetPlans();
    
    int GetBuildingDir();
    void SetBuildingDir(char buildingDir);

    int GetMoneyAmount() const;

    int GetMoneyTime() const;
    void SetMoneyTime(int newTime);

    float GetMoneyDelay() const;
};