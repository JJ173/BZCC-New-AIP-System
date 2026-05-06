#include "AIPBuildPlans.h"

#include <format>
#include <ranges>
#include <string>

// ==================================================
// BuilderPlan
// ==================================================

void BuilderPlan::Init(const char* cfg, const char* section)
{
	BuildConsClass[0] = 0;

	GetODFBool(cfg, section, "buildIfNoIdle", &BuildIfNoIdle, false);
	GetODFString(cfg, section, "consClass", sizeof(BuildConsClass), BuildConsClass, BuildConsClass);

	for (int i : std::views::iota(1))
	{
		std::string typeField = std::format("buildType{}", i);

		BuildInfo info;
		info.BuildState = BUILD_CHECK;

		if (!GetODFString(cfg, section, typeField.c_str(), sizeof(info.ObjClass), info.ObjClass, ""))
		{
			break;
		}

		std::string countField = std::format("buildCount{}", i);
		GetODFInt(cfg, section, countField.c_str(), &info.Count, 1);

		BuildList.push_back(std::move(info));
	}
}

bool BuilderPlan::Execute()
{
	if (DidConditionsFail())
	{
		return true;
	}

	if (BuildHandle != 0)
	{
		Stop(BuildHandle, 0);
		BuildHandle = 0;
		BuildId = 0;
	}

	std::vector<BuildInfo>::iterator i;

	for (i = BuildList.begin(); i != BuildList.end(); ++i)
	{
		BuildInfo& buildInfo = *i;

		if (!Execute(buildInfo))
		{
			return false;
		}
	}

	return true;
}

// ==================================================
// BuildMimimums
// ==================================================

void BuildMinimums::Init(const char* cfg, const char* section)
{
	BuilderPlan::Init(cfg, section);
}

bool BuildMinimums::Execute(BuildInfo& buildInfo)
{
	StartBuild(buildInfo.ObjClass);
	return true;
}