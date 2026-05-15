#pragma once

#include <ScriptUtils.h>
#include "Enums.hpp"
#include "Producer.hpp"

struct BuildRequest
{
    int Id;
    int Team;
    int Cost;
    Producer* AssignedProducer;
    std::string ObjOdf;
    Enums::BuildRequestStage Stage;
    Handle BuildHandle;
    long StartTime;
    int Priority;

    BuildRequest()
    {
        Id = 0;
        Team = 0;
        Cost = 0;
        AssignedProducer = nullptr;
        ObjOdf = "";
        Stage = Enums::BuildRequestStage::BUILD_TRY;
        BuildHandle = 0;
        StartTime = 0;
        Priority = 0;
    }
};