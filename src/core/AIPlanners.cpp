#include "AIPlanners.hpp"
#include "BuilderPlans.hpp"
#include "CollectorPlans.hpp"

int AIPlanners::GroupSize(std::vector<std::unique_ptr<AIPSchedPlan>>* plans, int startSlot)
{
    int priority = plans->at(startSlot)->Priority;
    int slot;

    for (slot = startSlot; slot < Constants::MAX_PLANS; ++slot)
    {
        if (slot >= plans->size())
        {
            break;
        }

        AIPSchedPlan* plan = plans->at(slot).get();

        if (plan == nullptr)
        {
            break;
        }

        if (plan->Priority != priority)
        {
            break;
        }
    }

    return slot - startSlot;
}

bool AIPlanners::SchedCompare(const AIPSchedPlan* p1, const AIPSchedPlan* p2)
{
    int priority1 = -1, priority2 = -1;
    
    if (p2 != nullptr)
    {
        priority2 = p2->Priority;
    }
    
    if (p1 != nullptr)
    {
        priority1 = p1->Priority;
    }
    
    return priority1 > priority2;
}

void AIPlanners::ClearPlanItems(int team)
{
    for (int i = 0; i < MAX_TEAMS; ++i)
    {
        if (team >= 0 && i != team)
        {
            continue;
        }

        AIPGlobals::AttackTargets[i].clear();
        AIPGlobals::DefendTargets[i].clear();
        AIPGlobals::PriorityGroups[i].clear();
        AIPGlobals::TGTCenters[i] = Vector {0.f, 0.f, 0.f};
        AIPGlobals::NormalGroups[i] = PriorityGroup(0, Enums::NORMAL);
        AIPGlobals::ConstructorReserveScrap[i] = 0;
        AIPGlobals::ConstructorReservePriority[i] = 0;
        AIPGlobals::ConstructorReserveTime[i] = 0;
    }
}

void AIPlanners::SetPlan(const char* aipCfg, const int teamNum)
{
    if (teamNum < 0 || teamNum >= MAX_TEAMS)
    {
        return;
    }
            
    if (GetCurWorld() != Constants::WORLD_LOCKSTEP)
    {
        PrintConsoleMessage("** Multiworld is on. Can't change AIPs from console **");
        return;
    }
        
    TeamOverwatch* thisTeam = &AIPGlobals::TeamOverwatchState[teamNum];

    if (aipCfg == nullptr || aipCfg[0] == '\0')
    {
        exu2::PrintConsoleMessage("SetPlan received an empty AIP file. Team: {} is now free.", teamNum);
        ClearPlanItems(teamNum);
        thisTeam->ClearPlans();
        return;
    }
        
    if (strcmp(AIPGlobals::LastPlanFile[teamNum], aipCfg) == 0)
    {
        // TRACE((team, " *** NOT SETTING %s FOR %d -- already running!", cfg, team));
        exu2::PrintConsoleMessage("{} is already running for team {}.", aipCfg, teamNum);
        return;
    }
        
    exu2::PrintConsoleMessage("SetPlan({}, {})", aipCfg, teamNum);
        
    ClearPlanItems(teamNum);
    thisTeam->ClearPlans();
    
    strncpy_s(AIPGlobals::LastPlanFile[teamNum], aipCfg, _TRUNCATE);
    strncpy_s(AIPGlobals::LastAIPFileForDLL[teamNum], aipCfg, _TRUNCATE);
    
    if (AIPGlobals::AIPDebugTeam < 0 || AIPGlobals::AIPDebugTeam == teamNum)
    {
        strncpy_s(AIPGlobals::LastAIPFile, aipCfg, _TRUNCATE);
        
        for (size_t ii = 0; ii < strlen(aipCfg); ii++)
        {
            if (islower(AIPGlobals::LastAIPFile[ii]))
            {
                AIPGlobals::LastAIPFile[ii] = static_cast<char>(toupper(AIPGlobals::LastAIPFile[ii]));   
            }
        }
    }
    
    if (!OpenODF(aipCfg))
    {
        exu2::PrintConsoleMessage("AIP ERROR: AIP file {} not found!", aipCfg);
        return;
    }

    thisTeam->Setup(teamNum);
    thisTeam->LoadTuning(aipCfg);
    thisTeam->FirstLoad();
    thisTeam->LoadMatches();
    thisTeam->LoadPlanNames();
    thisTeam->LoadScheduledPlans();

    // TODO: Ask VT about "LuaManager" and see if it's available. If not, I think we can easily work around this
    // based on the context.

    // Clean up.
    CloseODF(aipCfg);

    std::sort(thisTeam->GetPlans()->begin(), thisTeam->GetPlans()->end(), 
        [](const std::unique_ptr<AIPSchedPlan>& p1, const std::unique_ptr<AIPSchedPlan>& p2) {
            return SchedCompare(p1.get(), p2.get());
        });

    AIPGlobals::LastAIPTeam = teamNum;
}

void AIPlanners::ExecuteSchedPlans()
{
    if (GetCurWorld() != Constants::WORLD_LOCKSTEP)
    {
        return;
    }

    // No short-circuiting needed yet.
    AIPGlobals::BreakAllAIPProcessing = false;

    // Bail if we can't run yet.
    if (AIPGlobals::CurrentGameTurn < AIPGlobals::NextPlanExecuteTurn)
    {
        return;
    }

    // Set the next delay using the weird method from the original code...
    int gameTPS = SecondsToTurns(1.0f); // Grabs the TPS.
    AIPGlobals::NextPlanExecuteTurn = ((AIPGlobals::CurrentGameTurn + gameTPS + gameTPS - 1) / gameTPS) * gameTPS;

    bool updateTargets = false;

    if (AIPGlobals::CurrentGameTurn >= AIPGlobals::TargetUpdateTurn)
    {
        updateTargets = true;
        AIPGlobals::TargetUpdateTurn = AIPGlobals::CurrentGameTurn + 10 * gameTPS;
    }

    // Run through each team and see if it has any active plans. If not, don't process.
    for (int team = 0; team < MAX_TEAMS; ++team)
    {
        // Grab the team overwatch for this team.
        TeamOverwatch* teamOverwatch = &AIPGlobals::TeamOverwatchState[team];

        // Something is wrong if we don't have this so consider this an invalid team.
        if (teamOverwatch == nullptr)
        {
            continue;
        }

        auto* plans = teamOverwatch->GetPlans();

        // No plans? Bail.
        if (plans->empty())
        {
            continue;
        }

        teamOverwatch->UpdateBuilds(true);

        if (updateTargets)
        {
            AIPSchedPlan::UpdateTargets(team);
        }

        // Add cheat money.
        int moneyAmount = teamOverwatch->GetMoneyAmount();
        int moneyTime = teamOverwatch->GetMoneyTime();

        if (moneyAmount > 0 && AIPGlobals::CurrentGameTurn > moneyTime)
        {
            AddScrap(team, moneyAmount);
            teamOverwatch->SetMoneyTime(AIPGlobals::CurrentGameTurn + SecondsToTurns(teamOverwatch->GetMoneyDelay()));
        }

        // Don't let Constructors hoard scrap for too long.
        if (AIPGlobals::ConstructorReserveScrap[team] && AIPGlobals::CurrentGameTurn >= AIPGlobals::ConstructorReserveTime[team] + 300)
        {
            AIPGlobals::ConstructorReserveScrap[team] = 0;
            AIPGlobals::ConstructorReservePriority[team] = 0;
            AIPGlobals::ConstructorReserveTime[team] = 0;
        }

        // TODO: Recode this to handle multiple teams since this can only handle one, 
        // but I want to get it working for now. 
        // An idea could be to insert enemy team numbers into the TeamOverwatch class 
        // and cycle through that when looking for conditions.
        int hisTeam = -1;

        // Find a team with a player on it.
        for (int j = 1; j < MAX_TEAMS; j++)
        {
            // Check if a player exists on this team.
            Handle pHandle = GetPlayerHandle(j);

            if (pHandle != 0)
            {
                hisTeam = j;
                break;
            }
        }

        // Fall back.
        if (hisTeam < 0)
        {
            if (team == 6)
            {
                hisTeam = 1;
            }
            else 
            {
                hisTeam = 6;
            }
        }

        // Grab the scarp totals.
        int maxScrap = GetMaxScrap(team);
        int hisMaxScrap = GetMaxScrap(hisTeam);
        int maxPower = GetMaxPower(team);
        int hisMaxPower = GetMaxPower(team);

        // Start executing plans for this team.
        size_t planSlot = 0;
        int lastPriority = 0x7FFFFFFF;
        bool didAnyFail = false;

        // Run through each slot.
        // TODO: Possibly re-write this to use the # of plans stored within the team otherwise 
        // this could get expensive when multiple AIPs are being set.
        while (planSlot < plans->size())
        {
            AIPSchedPlan* plan = plans->at(planSlot).get();

            if (plan == nullptr)
            {
                break;
            }

            // If any plans in the last set fail, and we moved to the next priority, then quit.
            if (didAnyFail && plan->Priority < lastPriority)
            {
                break;
            }

            // Check if moving into the new set of priorities, and reset the fail indicator if so.
            if (plan->Priority < lastPriority)
            {
                lastPriority = plan->Priority;
                didAnyFail = false;
            }

            int curScrap = GetScrap(team);
            int hisCurScrap = GetScrap(hisTeam);
            int curPower = GetPower(team);
            int hisCurPower = GetPower(hisTeam);

            // Execute group of plans.
            bool isSatisfied = false;
            {
                // Get the group type.
                PriorityGroup& group = AIPSchedPlan::FindGroup(team, plan->Priority);

                // Switch the group type.
                switch (group.Type)
                {
                    case Enums::GroupType::NORMAL:
                    {
                        if (plan->ScrapSatisfied(curScrap, maxScrap)
                            && plan->PowerSatisfied(curPower, maxPower)
                            && plan->HisScrapSatisfied(hisCurScrap, hisMaxScrap)
                            && plan->HisPowerSatisfied(hisCurPower, hisMaxPower))
                        {
                            isSatisfied = plan->Execute();

                            if (plan->ContinueEvenIfFail)
                            {
                                isSatisfied = true;
                            }

                            if (AIPGlobals::BreakAllAIPProcessing)
                            {
                                return; // BAIL, NOW! AIPs just switched, all pointers are invalid
                            }
                        }
                        else
                        {
                            isSatisfied = true;
                        }

                        ++planSlot;
                        break;
                    }
                    case Enums::GroupType::LOOP:
                    {
                        int groupSize = GroupSize(plans, (int)planSlot);
                       
                        if (group.InProgress)
                        {
                            if (plan->ScrapSatisfied(curScrap, maxScrap)
                                && plan->PowerSatisfied(curPower, maxPower)
                                && plan->HisScrapSatisfied(hisCurScrap, hisMaxScrap)
                                && plan->HisPowerSatisfied(hisCurPower, hisMaxPower))
                            {
                                if (plan->Execute())
                                {
                                    if (AIPGlobals::BreakAllAIPProcessing)
                                    {
                                        return; // BAIL, NOW! AIPs just switched, all pointers are invalid
                                    }

                                    if (++group.CurPlan == groupSize)
                                    {
                                        group.InProgress = false;
                                        group.CurPlan = 0;
                                    }
                                }
                                else if (AIPGlobals::BreakAllAIPProcessing)
                                {
                                    return; // BAIL, NOW! AIPs just switched, all pointers are invalid
                                }
                            }

                            isSatisfied = false;
                        }
                        else
                        {
                            if (plan->ScrapSatisfied(curScrap, maxScrap)
                                && plan->PowerSatisfied(curPower, maxPower)
                                && plan->HisScrapSatisfied(hisCurScrap, hisMaxScrap)
                                && plan->HisPowerSatisfied(hisCurPower, hisMaxPower))
                            {
                                if (plan->Execute())
                                {
                                    if (AIPGlobals::BreakAllAIPProcessing)
                                    {
                                        return; // BAIL, NOW! AIPs just switched, all pointers are invalid
                                    }

                                    isSatisfied = true;
                                    planSlot += groupSize;
                                    break;
                                }

                                if (AIPGlobals::BreakAllAIPProcessing)
                                {
                                    return; // BAIL, NOW! AIPs just switched, all pointers are invalid
                                }
                            }

                            group.InProgress = true;
                            group.CurPlan = 0;
                        }

                        isSatisfied = false;
                        break;
                    }
                    case Enums::GroupType::SELECT:
                    {
                        // Deprecated.
                        break;
                    }
                }
            }

            if (!isSatisfied)
            {
                didAnyFail = true;
            }
        }

        if (planSlot >= plans->size())
        {
            for (auto& plan : *plans)
            {
                if (plan == nullptr)
                {
                    break;
                }

                plan->ResetTries();
            }
        }

        // If we failed to finish given priority level, then ask all the lower
        // priority plans to release all finished units...
        // (this prevents deadlock of lower priority completed, but could not release
        // because higher priority is waiting for them)
        // Currently only the attack plans check if attackers finished and makes units idle if so...
        if (lastPriority != 0x7FFFFFFF)
        {
            AIPSchedPlan::ReleaseFinishedUnits(team, lastPriority);
        }
    }
}