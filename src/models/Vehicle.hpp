#pragma once

#include "Enums.hpp"
#include <ScriptUtils.h>

struct Vehicle
{
    int IdleTime;
    int Team;
    int Type;
    std::string ODF;
    std::string Class;
    Handle Handle;
    bool IsCraft;
    Vector LastPosition;
};