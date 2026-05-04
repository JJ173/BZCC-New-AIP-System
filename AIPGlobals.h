#pragma once

#include <cstdint>
#include <ScriptUtils.h>
#include <utility>
#include <vector>

class TeamOverwatch;
class AIPSchedPlan;

namespace AIPGlobals
{
    constexpr auto StartString = "Start";
    constexpr auto IdleDispatcherString = "IdleDispatcher";

    constexpr int MAX_PLAN_CONDITIONS = 16;
    constexpr int MAX_UNITS_PER_TEAM = 128;
    constexpr int MAX_PLANS = 1024;
    constexpr int MAX_POSSIBLE = 1024;
    constexpr int MAX_RESPONSES = 8;
    constexpr int MAX_STAGE_POINTS = 32;

    // Unsure what the MAX_WORLDS const is from context. Perhaps 3 given what Ken explained with lockstep.
    // For now, MAX_WORLDS has been set to 3. We'll adjust as needed if something breaks.
    constexpr int MAX_WORLDS = 3;

    constexpr int WORLD_LOCKSTEP = 0;

    constexpr float BIRTH_TIME_IDLE = 10.0f;
    constexpr float SAFE_SCRAP_DISTANCE = 150.0f * 150.0f;
    constexpr float IdleDistSq = 8.0f * 8.0f;
    constexpr float BaseDistSq = 75.0f * 75.0f;

    constexpr float MIN_KEEPATTACK_AMMO = 0.02f;
    constexpr float MIN_SEND_AMMO = 0.10f;
    constexpr float MIN_BUILD_AMMO = 0.02f;
    constexpr float MIN_ATTACK_AMMO = 0.10f;

    constexpr float MIN_WRECK_AMMO = 0.10f;
    constexpr float MIN_WRECK_HEALTH = 0.25f;
    constexpr float MAX_WRECK_AMMO = 0.10f;
    constexpr float MAX_WRECK_HEALTH = 0.25f;
    constexpr float MAX_REPAIR_TIME = 20.0f;
    constexpr float MAX_WRECK_DIST = 30.0f;

    constexpr float OLD_ENEMY_SEARCH_RADIUS = 400.0f;

    constexpr float MAX_HOLD_TIME = 20.0f;
    constexpr float MIN_HOLD_DIST = 10.0f;
    constexpr float MAX_HOLD_DIST = 100.0f;

    enum GroupType : uint8_t
    {
        NORMAL,
        LOOP,
        SELECT
    };

    enum VehicleType : uint8_t
    {
        SCAV,
        TURRET,
        CONSTRUCTOR,
        GENERAL,
        SERVICE_TRUCK,
        BOMBER
    };

    enum TTB_RESULT : uint8_t
    {
        BADTEAM,
        NOPRODUCER,
        PRODUCERBUSY,
        OVERMAXSCRAP,
        OVERCURSCRAP,
        OVERRESERVESCRAP,
        REQUIREMENTSFAIL,

        BUILDING,
    };

    enum IfConditionTeamFilter : uint8_t
    {
        IFCONDITION_SAMETEAM,
        IFCONDITION_FRIENDLY,
        IFCONDITION_ENEMY,
        IFCONDITION_MAX
    };

    enum CompareType : uint8_t
    {
        COMPARE_EQUAL,
        COMPARE_NOTEQUAL,
        COMPARE_LESS,
        COMPARE_LESSEQUAL,
        COMPARE_GREATER,
        COMPARE_GREATEREQUAL,
        COMPARE_MAX
    };

    enum PickTargetType : uint8_t
    {
        PickTarget_MINIMUM,
        PickTarget_FirstSeqNo = PickTarget_MINIMUM,
        PickTarget_Closest,
        PickTarget_Random,
        PickTarget_MAXIMUM
    };

    struct GameObjectInfo
    {
        int IdleTime;
        int Team;
        const char* ODF;
        const char* Class;
        Handle Handle;
        bool IsCraft;
        Vector LastPosition;
        VehicleType Type;
    };

    struct PossibleTargetInfo
    {
        Handle Handle;
        Vector Where;
    };

    struct PriorityGroup
    {
        int Priority;
        GroupType Type;
        bool InProgress;
        int CurPlan;

        PriorityGroup(int priority, GroupType groupType)
        {
            Priority = priority;
            Type = groupType;
            InProgress = false;
            CurPlan = 0;
        }

        PriorityGroup()
        {
            Priority = 0;
            Type = NORMAL;
            InProgress = false;
            CurPlan = 0;
        }
    };

    struct BuildRequest
    {
        int Id;
        int Team;
        int Slot;
        // GameObjectClass (might need to use const char for this one).
        const char* ObjClass;
        int Stage;
        Handle Handle;
        long StartTime;
        int Priority;

        BuildRequest()
        {
            Id = 0;
            Team = 0;
            Slot = 0;
            ObjClass = nullptr;
            Stage = 0;
            Handle = 0;
            StartTime = 0;
            Priority = 0;
        }
    };

    struct MatchResponseInfo
    {
        int AttackerCount;
        std::vector<const char*> Attackers;

        MatchResponseInfo()
        {
            AttackerCount = 0;
            Attackers.clear();
        }
    };

    struct MatchTargetInfo
    {
        unsigned long TargetBaseODFCrc;
        char TargetBaseCfg[ODF_MAX_LEN] = {};

        std::vector<MatchResponseInfo> Responses;

        MatchTargetInfo()
        {
            TargetBaseODFCrc = 0;
            TargetBaseCfg[0] = 0;
        }
    };

    struct MatchInfo
    {
        std::vector<MatchTargetInfo> Targets;
    };

    struct SchedTeam
    {
        char AIPName[ODF_MAX_LEN] = {};
        const char* ScavClass;
        const char* ConsClass;
        int MoneyAmount;
        int MoneyDelay;
        long MoneyTime;

        SchedTeam()
        {
            AIPName[0] = 0;
            ScavClass = nullptr;
            ConsClass = nullptr;
            MoneyAmount = 0;
            MoneyDelay = 0;
            MoneyTime = 0;
        }
    };

    extern SchedTeam SchedTeams[MAX_TEAMS];

    typedef std::vector<Handle> HandleList;
    typedef std::vector<unsigned long> ObjClassCRCList;

    extern void SaveBuildRequests();
    extern void LoadBuildRequests();
    extern void PostLoadBuildRequests();

    extern int SubmitBuild(int team, const char* objClass, int priority);
    extern int BuilderSlot(int team, const char* objClass);

    extern bool CheckProvides(Handle handle, const char* objClass, bool checkProvides);
    extern bool NeedsTap(Handle handle);

    inline float GridsPerMeter = 0.0f;
    inline Vector CompareOrigin;

    inline std::vector<GameObjectInfo>& GetAllGameObjects();
    inline std::vector<GameObjectInfo>& GetAllVehicles();

    inline int TeamBuildingDir[MAX_TEAMS];
    inline int ConstructorReserveScrap[MAX_TEAMS];
    inline int ConstructorReservePriority[MAX_TEAMS];
    inline int ConstructorReserveTime[MAX_TEAMS];

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
    inline std::vector<BuildRequest> BuildRequests[MAX_TEAMS];

    inline char LastPlanFile[MAX_TEAMS][ODF_MAX_LEN];
    inline char LastAIPFileForDLL[MAX_TEAMS][ODF_MAX_LEN];

    inline AIPSchedPlan* SchedPlan[MAX_TEAMS][MAX_PLANS];
    inline SchedTeam SchedTeams[MAX_TEAMS];

    extern TeamOverwatch TeamOverwatchState[MAX_TEAMS];
}

namespace AIPGlobalUtils
{
    static bool IsNull2DVector(const VECTOR_2D& vector)
    {
        return vector.x == 0.0f && vector.z == 0.0f;
    }
    
    static VECTOR_2D Get2DVectorFromPath(const char* path, const int point)
    {
        VECTOR_2D retVal = {0, 0};
        size_t bufSize = 0;

        if (path)
        {
            GetPathPoints(path, bufSize, nullptr);
        }

        if (!bufSize || point < 0)
        {
            return retVal;
        }

        if (std::cmp_less(point, bufSize))
        {
            auto pData = static_cast<float*>(_alloca(sizeof(float) * 2 * bufSize));
            if (GetPathPoints(path, bufSize, pData))
            {
                retVal = VECTOR_2D(pData[2 * point + 0], pData[2 * point + 1]);
            }
        }

        return retVal;
    }

    static float GetDistance2DSquared(const Vector& v1, const Vector& v2)
    {
        return (v2.x - v1.x) * (v2.x - v1.x) + (v2.z - v1.z) * (v2.z - v1.z);
    }

    template <class T>
    static T Clamp(const T x, const T min, const T max)
    {
        if (x < min)
        {
            return min;
        }

        if (x > max)
        {
            return max;
        }

        return x;
    }
    
    static bool ComparePoints(const Vector& a, const Vector& b)
    {
        const float d1 = GetDistance2DSquared(a, AIPGlobals::CompareOrigin);
        const float d2 = GetDistance2DSquared(b, AIPGlobals::CompareOrigin);
        return d1 < d2;
    }
    
    static bool IsRecycler(const char* objClass)
    {
        return strcmp(objClass, "CLASS_RECYCLERVEHICLE") == 0 || strcmp(objClass, "CLASS_RECYCLERVEHICLEH") == 0;
    }
}
