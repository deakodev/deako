#include "Handle.h"
#include "dkpch.h"

#include <random>

namespace Deako {

    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_Engine(s_RandomDevice());
    static std::uniform_int_distribution<DkU64> s_UniformDistribution;

    DkHandle::DkHandle()
        : m_Handle(s_UniformDistribution(s_Engine))
    {
    }

    DkHandle::DkHandle(DkU64 handle)
        : m_Handle(handle)
    {
    }

}
