#pragma once

#include <map>
#include <string>

#include "Constants.hpp"

namespace Maps
{
    inline std::map<std::string, std::string> PRODUCER_HEADERS =
    {
        { Constants::CLASS_ARMORY, "ArmoryClass"},
        { Constants::CLASS_CONSTRUCTIONRIG, "ConstructionRigClass"},
        { Constants::CLASS_CONSTRUCTIONRIGT, "ConstructionRigTClass"},
        { Constants::CLASS_RECYCLER, "FactoryClass"},
        { Constants::CLASS_FACTORY, "FactoryClass"}
    };
}