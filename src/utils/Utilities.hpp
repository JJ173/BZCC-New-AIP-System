#pragma once

#include <string>
#include <format>
#include <functional>

#include "AIPGlobals.hpp"
#include "Constants.hpp"

namespace Utilities
{
    using namespace AIPGlobals;

    static bool IsNull2DVector(const VECTOR_2D& vector)
    {
        return vector.x == 0.0f && vector.z == 0.0f;
    }

    static VECTOR_2D Get2DVectorFromPath(const char* path, const int point)
    {
        VECTOR_2D retVal = { 0, 0 };
        size_t bufSize = 0;

        if (path)
        {
            GetPathPoints(path, bufSize, nullptr);
        }

        if (!bufSize || point < 0)
        {
            return retVal;
        }

        if (std::cmp_less(point, bufSize))
        {
            auto pData = static_cast<float*>(_malloca(sizeof(float) * 2 * bufSize));

            if (pData == nullptr)
            {
                return retVal;
            }

            if (GetPathPoints(path, bufSize, pData))
            {
                retVal = VECTOR_2D(pData[2 * point + 0], pData[2 * point + 1]);
            }
        }

        return retVal;
    }

    template<typename V1, typename V2>
    inline float GetDistance2DSquared(const V1& v1, const V2& v2)
    {
        return (v2.x - v1.x) * (v2.x - v1.x) + (v2.z - v1.z) * (v2.z - v1.z);
    }

    template <class T>
    static T Clamp(const T x, const T min, const T max)
    {
        if (x < min)
        {
            return min;
        }

        if (x > max)
        {
            return max;
        }

        return x;
    }

    static bool ComparePoints(const Vector& a, const Vector& b)
    {
        const float d1 = GetDistance2DSquared(a, CompareOrigin);
        const float d2 = GetDistance2DSquared(b, CompareOrigin);
        return d1 < d2;
    }

    static bool IsRecycler(const std::string& objClass)
    {
        return objClass == Constants::CLASS_RECYCLERVEHICLE || objClass == Constants::CLASS_RECYCLERVEHICLEH || objClass == Constants::CLASS_RECYCLER;
    }

    template <class T>
    static T GetODFValueFromChain(std::string odf_path, std::string section, std::string key)
    {
        static char result[ODF_MAX_LEN];
        result[0] = '\0';

        std::function<T(std::string, std::string, std::string, int)> SearchODFChain;

        SearchODFChain = [&](std::string current_odf, std::string sec, std::string k, int depth)->T
            {
                if (current_odf.empty() || depth <= 0)
                {
                    return T{};
                }

                if (!OpenODF(current_odf.c_str()))
                {
                    return T{};
                }

                T value{};
                bool success = false;

                if constexpr (std::is_same_v<T, int>)
                {
                    int tempVal = 0;
                    success = GetODFInt(current_odf.c_str(), sec.c_str(), k.c_str(), &tempVal);
                    value = tempVal;
                }
                else if constexpr (std::is_same_v<T, float>)
                {
                    float tempVal = 0.0f;
                    success = GetODFFloat(current_odf.c_str(), sec.c_str(), k.c_str(), &tempVal);
                    value = tempVal;
                }
                else if constexpr (std::is_same_v<T, std::string>)
                {
                    char tempStr[ODF_MAX_LEN] = {};
                    success = GetODFString(current_odf.c_str(), sec.c_str(), k.c_str(), sizeof(tempStr), tempStr);
                    value = tempStr;
                }
                else
                {
                    static_assert(sizeof(T) == 0, "GetODFValueFromChain: Unsupported type T");
                }

                if (success)
                {
                    return value;
                }

                return SearchODFChain(nullptr, sec, k, depth - 1);
            };

        return SearchODFChain(odf_path, section, key, 10);
    }
}