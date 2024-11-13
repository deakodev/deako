#pragma once

#include "Deako/Math/Math.h"

namespace Deako {

    class DkHandle
    {
    public:
        DkHandle();
        DkHandle(DkU64 handle);
        DkHandle(const DkHandle&) = default;

        operator DkU64() const { return m_Handle; }
    private:
        DkU64 m_Handle;
    };

}

namespace std {

    template<>
    struct hash<Deako::DkHandle>
    {
        std::size_t operator()(const Deako::DkHandle& handle) const
        {
            return (Deako::DkU64)handle;
        }
    };

}
