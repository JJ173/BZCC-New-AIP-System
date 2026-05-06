#pragma once

#include "AIPSchedPlan.h"

class CollectField : public AIPSchedPlan
{

};

class CollectPool : public AIPSchedPlan
{
public:
	virtual const char* GetPlanName()
	{
		return "CollectPool";
	}

	enum CollectState
	{
		FIND_POOL,
		BUILD_SCAV,
		SEND_SCAV
	};

	CollectState state = FIND_POOL;
	Handle ScavHandle = 0;
	Handle DepositHandle = 0;

	int MaxTimes = 0;
	int TriedTimes = 0;

	const char* BuildScavClass = nullptr;
	bool CheckProvides = AIPGlobals::DefaultCheckProvides[Team];

	void Init(const char* cfg, const char* section);

	virtual void ResetTries()
	{
		TriedTimes = 0;
	}

	// Handle has been claimed by another plan, with the pThis
	// pointer. Plan needs to release the handle.
	virtual void ReleaseHandle(void* pThis, int handle)
	{
	}

	//virtual void Save(ILoadSaveVisitor& visitor);
	//virtual void Load(ILoadSaveVisitor& visitor);
	//virtual void PostLoad(ILoadSaveVisitor& visitor);

	bool Execute();

	virtual void Done()
	{
		if (ScavHandle != 0)
		{
			Stop(ScavHandle, 0);
		}
	}
};