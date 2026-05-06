#pragma once

#include "AIPSchedPlan.h"

class BuilderPlan : public AIPSchedPlan
{
public:
	enum BuildState : uint8_t
	{
		BUILD_CHECK,
		BUILD_FINDSPOT,
		BUILD_SUBMIT,
		BUILD_COMMIT,
		BUILD_WAIT
	};

	char BuildConsClass[ODF_MAX_LEN] = {};

	struct BuildInfo
	{
		char ObjClass[64] = {};
		int Count = 0;
		BuildState BuildState = BUILD_CHECK;
		int NeedsPower = 0;
	};

	std::vector<BuildInfo> BuildList;

	BuilderPlan(int team) : AIPSchedPlan(team)
	{
		BuildConsClass[0] = 0;
	}

	void Init(const char* cfg, const char* section);
	bool Execute();

	virtual bool Execute(BuildInfo& info)
	{
		return true;
	}

	virtual void Done()
	{
		// Units handled by this plan get released by CancelAllBuilds()
	}
};

class BuildMinimums : public BuilderPlan
{
public:
	BuildMinimums(int team) : BuilderPlan(team)
	{

	}

	virtual const char* GetPlanName()
	{
		return "BuildMinimums";
	}

	virtual void ResetTries()
	{

	}

	// Handle has been claimed by another plan, with the pThis
	// pointer. Plan needs to release the handle.
	virtual void ReleaseHandle(void* pThis, int handle)
	{
	}

	void Init(const char* cfg, const char* section);
	bool Execute(BuildInfo& buildInfo);
};

class BuildLoop : public BuilderPlan
{
	virtual const char* GetPlanName()
	{
		return "BuildLoop";
	}

	int Outer;
	int Inner;
	int Loops;
	int MaxLoops;
	int LoopDelay;
	bool DelayActive;
	long TimeOut;

	BuildLoop(int team) : BuilderPlan(team)
	{
		Outer = 0;
		Inner = 0;
		Loops = 0;
		MaxLoops = 0;
		LoopDelay = 0;
		DelayActive = false;
		TimeOut = 0;
	}

	virtual void Save();
	virtual void Load();
	virtual void PostLoad()
	{
		BuilderPlan::PostLoad();
	}

	virtual void ResetTries()
	{

	}

	// Handle has been claimed by another plan, with the pThis
	// pointer. Plan needs to release the handle.
	virtual void ReleaseHandle(void* pThis, int handle)
	{
	}

	void Init(const char* cfg, const char* section);

	bool Execute(BuildInfo& info);
	bool Execute(void);
};