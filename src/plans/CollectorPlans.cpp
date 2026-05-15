#include "CollectorPlans.hpp"
#include "TeamOverwatch.hpp"

// ==================================================
// CollectorPlan
// ==================================================

void CollectorPlan::Init(std::string cfg, std::string section)
{
	BuildScavClass[0] = 0;

	const char* cfgCString = cfg.c_str();
	const char* sectionCString = section.c_str();

	GetODFBool(cfgCString, sectionCString, "buildIfNoIdle", &BuildIfNoIdle, false);
	GetODFBool(cfgCString, sectionCString, "CheckProvides", &CheckProvides, CheckProvides);
	GetODFString(cfgCString, sectionCString, "scavClass", sizeof(BuildScavClass), BuildScavClass, BuildScavClass);
	GetODFInt(cfgCString, sectionCString, "maxTimes", &MaxTimes, 0);

	// Fallback to "MaxTries" if MaxTimes is not found or has been set to 0.
	// Worth documenting, as a modder may want this to run infinitely, instead of being caught out.
	if (MaxTimes == 0)
	{
		GetODFInt(cfgCString, sectionCString, "maxTries", &MaxTimes, 0);
	}

	State = FIND_RESOURCE;
}


// ==================================================
// CollectField
// ==================================================

void CollectField::Init(std::string cfg, std::string section)
{
	CollectorPlan::Init(cfg, section);
}

bool CollectField::Execute()
{
	if (MaxTimes > 0 && TriedTimes >= MaxTimes)
	{
		return true;
	}

	if (DidConditionsFail())
	{
		return true;
	}

	return false;
}

// ==================================================
// CollectPool
// ==================================================

void CollectPool::Init(std::string cfg, std::string section)
{
	CollectorPlan::Init(cfg, section);
}

bool CollectPool::Execute()
{
	if (MaxTimes > 0 && TriedTimes >= MaxTimes)
	{
		return true;
	}

	if (DidConditionsFail())
	{
		return true;
	}

	if (!CheckBuild())
	{
		return false;
	}

	// TODO: Re-add this if we get a chance to grab this information from Exus.
	std::string UseScavClass(BuildScavClass);
	
	if (UseScavClass.empty())
	{
		UseScavClass = MyTeamOverwatch->ScavClass;
	}

	if (UseScavClass.empty())
	{
		// exu2::PrintConsoleMessage("AIP Error: CollectPool with priority {} has no valid ScavClass to use! Skipping...", &this->Priority);
		return true;
	}

	const char* scavClassCString = UseScavClass.c_str();
	int scavMask = 0;
	int scavMatch = 0;
	int scavProvides = 0;

	// Open the Scavenger ODF, and grab the relevant values.
	//if (!OpenODF(scavClassCString))
	//{
	//	// exu2::PrintConsoleMessage("AIP Error: CollectPool with priority {} failed to open {}, please provide a valid ODF. Skipping...", &this->Priority, UseScavClass);
	//	return true;
	//}

	// Grab the Scav AI process from the ODF to see what type of Scav we're dealing with.
	char ScavAIType[ODF_MAX_LEN] = {};
	GetODFString(scavClassCString, "GameObjectClass", "AIName", sizeof(ScavAIType), ScavAIType, "ScavProcess");

	std::string classBlock = "ScavengerClass";
	if (strcmp(ScavAIType, "ScavHProcess") == 0)
	{
		classBlock = "ScavengerHClass";
	}

	const char* classBlockCStr = classBlock.c_str();
	GetODFInt(scavClassCString, classBlockCStr, "ScavClassMask", &scavMask, 0);
	GetODFInt(scavClassCString, classBlockCStr, "ScavClassMatch", &scavMatch, 0);
	GetODFInt(scavClassCString, classBlockCStr, "ScavClassProvides", &scavProvides, 0);

	switch (State)
	{
	case FIND_RESOURCE:
	{
		if (Objects::MapPools.empty())
		{
			return true;
		}

		Vector start{ 0.f, 0.f, 0.f };
		Deposit = FindNearestDeposit(scavMask, scavMatch, scavProvides, Team, start);

		if (Deposit == nullptr)
		{
			TriedTimes = 0;
			return true;
		}

		// Try and find an idle scav.
		Handle scav = 0;

		int numIdleScavs = MyTeamOverwatch->CountIdleScavs(BuildScavClass);
		if (numIdleScavs > 0)
		{
			scav = MyTeamOverwatch->ActivateIdleScavNear(start, BuildScavClass);

			if (scav)
			{
				start = GetPosition(scav);
			}
		}

		// Run the "slower scan" through idle units.
		if (scav == 0)
		{
			AIPGlobals::HandleList emptyList;
			scav = MyTeamOverwatch->FindIdleUnit(BuildScavClass, CheckProvides, emptyList);

			if (scav != 0)
			{
				start = GetPosition(scav);
			}
			else
			{
				// Find Recycler.
				Producer* recycler = MyTeamOverwatch->GetRecycler();

				if (recycler == nullptr)
				{
					return false;
				}

				start = GetPosition(recycler->ProdHandle);
			}
		}

		Deposit = FindNearestDeposit(scavMask, scavMatch, scavProvides, Team, start);

		if (Deposit == 0)
		{
			return true;
		}

		if (scav != 0)
		{
			ScavHandle = scav;
			LookAt(ScavHandle, Deposit->h, 0);
			State = SEND_SCAV;
			return true;
		}

		if (BuildIfNoIdle)
		{
			StartBuild(scavClassCString);
			State = BUILD_SCAV;
			return true;
		}

		// Waiting for an idle scav.
		return false;
	}
	case BUILD_SCAV:
	{
		Handle scav = 0;

		if (BuildHandle == 0 || (scav = GetLiveObject(BuildHandle)) == 0)
		{
			State = FIND_RESOURCE;
			BuildHandle = 0;
			break;
		}

		BuildHandle = 0;

		if (Deposit == nullptr)
		{
			ScavHandle = 0;
			Stop(scav, 0);
		}

		LookAt(ScavHandle, Deposit->h, 0);
		State = SEND_SCAV;

		return true;
	}
	}

	return false;
}