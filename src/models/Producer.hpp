#pragma once

#include <ScriptUtils.h>
#include <string>

#include "Constants.hpp"
#include "Utilities.hpp"

class Producer
{
public:
	Handle ProdHandle;

    std::string ObjODF;
	std::string ObjClass;
    
    bool IsRecycler;

    inline bool IsProducerAlive()
    {
        return IsAlive(ProdHandle);
    }

	inline bool IsProducerBusy()
	{
		return IsBusy(ProdHandle);
	}

	inline bool CanBuildItem(std::string objOdf)
	{
        const char* odfCString = ObjODF.c_str();

        std::string heading;
        std::string property;

        for (int i = 0; i <= Constants::MAX_BUILD_GROUPS; i++)
        {
            heading = "";

            if (i == 0)
            {
                heading = Maps::PRODUCER_HEADERS[ObjClass];
            }
            else
            {
                heading = std::format("BuildGroup{}", i);
            }

            for (int j = 1; j <= Constants::MAX_BUILD_SLOTS; j++)
            {
                property = std::format("buildItem{}", j);

                // Check to see if we can build this item (support inheritance)
                std::string result(Utilities::GetODFValueFromChain<std::string>(objOdf, heading, property));

                if (result == objOdf)
                {
                    return true;
                }
            }
        }

        return false;
	}

    inline void BuildItem(std::string objOdf)
    {
        Build(ProdHandle, objOdf.c_str(), 1);
    }
};