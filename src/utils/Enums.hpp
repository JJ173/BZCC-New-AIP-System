#pragma once

#include <cstdint>

namespace Enums
{
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

    enum BuildRequestStage : uint8_t
    {
        BUILD_TRY = 0,
        BUILD_COMMIT = 1,
        BUILD_WAIT = 2,
        BUILD_DONE = 3
    };
}