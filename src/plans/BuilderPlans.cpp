#include "BuilderPlans.hpp"
#include "TeamOverwatch.hpp"

// ==================================================
// BuilderPlan
// ==================================================

void BuilderPlan::Init(std::string cfg, std::string section)
{
	BuildConsClass[0] = 0;

	const char* cfgCString = cfg.c_str();
	const char* sectionCString = section.c_str();

	GetODFBool(cfgCString, sectionCString, "buildIfNoIdle", &BuildIfNoIdle, false);
	GetODFString(cfgCString, sectionCString, "consClass", sizeof(BuildConsClass), BuildConsClass, BuildConsClass);

	for (int i : std::views::iota(1))
	{
		std::string typeField = std::format("buildType{}", i);

		BuildInfo info;
		info.BuildState = BUILD_CHECK;

		if (!GetODFString(cfgCString, sectionCString, typeField.c_str(), sizeof(info.ObjClass), info.ObjClass, ""))
		{
			break;
		}

		std::string countField = std::format("buildCount{}", i);
		GetODFInt(cfgCString, sectionCString, countField.c_str(), &info.Count, 1);

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

void BuildMinimums::Init(std::string cfg, std::string section)
{
	BuilderPlan::Init(cfg, section);
}

bool BuildMinimums::Execute(BuildInfo& buildInfo)
{
	StartBuild(buildInfo.ObjClass);
	return true;
}