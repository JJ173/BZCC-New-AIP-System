#include <memory>
#include "AIPGlobals.hpp"
#include "TeamOverwatch.hpp"

std::unique_ptr<TeamOverwatch[]> AIPGlobals::TeamOverwatchState = std::make_unique<TeamOverwatch[]>(MAX_TEAMS);
