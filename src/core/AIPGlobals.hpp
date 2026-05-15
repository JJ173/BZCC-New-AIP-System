#pragma once

#include <memory>
#include <cstdint>
#include <ScriptUtils.h>
#include <utility>
#include <vector>
#include <string>
#include <map>

#include "Deposit.hpp"
#include "PriorityGroup.hpp"
#include "Constants.hpp"
#include "Maps.hpp"

class TeamOverwatch;
class AIPSchedPlan;

namespace AIPGlobals
{
    struct PossibleTargetInfo
    {
        Handle Handle;
        Vector Where;
    };

    struct MatchResponseInfo
    {
        int AttackerCount;
        std::vector<std::string> Attackers;

        MatchResponseInfo()
        {
            AttackerCount = 0;
            Attackers.clear();
        }
    };

    struct MatchTargetInfo
    {
        char TargetBaseODF[ODF_MAX_LEN] = {};
        char TargetBaseCfg[ODF_MAX_LEN] = {};

        std::vector<MatchResponseInfo> Responses;

        MatchTargetInfo()
        {
            TargetBaseODF[0] = 0;
            TargetBaseCfg[0] = 0;
        }
    };

    struct MatchInfo
    {
        std::vector<MatchTargetInfo> Targets;
    };

    typedef std::vector<Handle> HandleList;

    // Appears to be responsible for short-circuiting the ExecuteSchedPlans() method
    // in the event of something changing, such as a new AIP being loaded for the team.
    inline bool BreakAllAIPProcessing = false;

    inline float GridsPerMeter = 0.0f;
    inline Vector CompareOrigin;

    // Global delays and timers.
    inline int CurrentGameTurn = 0;
    inline int NextPlanExecuteTurn = 0;
    inline int TargetUpdateTurn = 0;
    
    inline int BuildId = 0;

    inline int TeamBuildingDir[MAX_TEAMS];
    inline int ConstructorReserveScrap[MAX_TEAMS];
    inline int ConstructorReservePriority[MAX_TEAMS];
    inline int ConstructorReserveTime[MAX_TEAMS];
    inline bool DefaultCheckProvides[MAX_TEAMS];

    inline int AIPDebugTeam;
    inline const char* FailedPlanClass;
    inline const char* FailedPlanExplanation;
    inline int FailedPlanPriority;
    inline int LastAIPTeam;
    inline char LastAIPFile[128];
    inline char BuildingStr[128];

    inline std::vector<PriorityGroup> PriorityGroups[MAX_TEAMS];
    inline PriorityGroup NormalGroups[MAX_TEAMS];
    inline Vector TGTCenters[MAX_TEAMS];
    
    inline std::vector<Handle> AttackTargets[MAX_TEAMS];
    inline std::vector<Handle> DefendTargets[MAX_TEAMS];

    inline char LastPlanFile[MAX_TEAMS][ODF_MAX_LEN];
    inline char LastAIPFileForDLL[MAX_TEAMS][ODF_MAX_LEN];

    extern std::unique_ptr<TeamOverwatch[]> TeamOverwatchState;
}