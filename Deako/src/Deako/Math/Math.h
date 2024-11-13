#pragma once

#include <glm/glm.hpp>

namespace Deako {

    // Integer
    typedef std::uint8_t  DkU8;
    typedef std::int8_t   DkS8;
    typedef std::uint16_t DkU16;
    typedef std::int16_t  DkS16;
    typedef std::uint32_t DkU32;
    typedef std::int32_t  DkS32;
    typedef std::uint64_t DkU64;
    typedef std::int64_t  DkS64;

    // Floating-point
    typedef float  DkF32;
    typedef double DkF64;

    // GLM vectors
    typedef glm::vec2 DkVec2;
    typedef glm::vec3 DkVec3;
    typedef glm::vec4 DkVec4;
    typedef glm::uvec4 DkUVec4;

    // GLM matricies
    typedef glm::mat4 DkMat4;

    inline DkF32 DkSaturate(DkF32 f) { return (f < 0.0f) ? 0.0f : (f > 1.0f) ? 1.0f : f; }
    #define DK_F32_TO_S8_SAT(_VAL) ((int)(DkSaturate(_VAL) * 255.0f + 0.5f))  

    inline DkF32 DkRoundDown(DkF32 x) { return (DkF32)((x >= 0 || (DkF32)(int)x == x) ? (int)x : (int)x - 1); }
    inline DkVec2 DkRoundDown(const DkVec2& vec) { return DkVec2(DkRoundDown(vec.x), DkRoundDown(vec.y)); }

    inline DkVec3 U32ToVec3(DkU32 in)
    {
        DkF32 scale = 1.0f / 255.0f;
        return DkVec3(
            ((in >> 16) & 0x0FF) * scale,
            ((in >> 8) & 0x0FF) * scale,
            ((in >> 0) & 0x0FF) * scale);
    }

    inline DkU32 Vec3ToU32(const DkVec3& in)
    {
        DkU32 out;
        out = ((DkU32)DK_F32_TO_S8_SAT(in.x)) << 16;
        out |= ((DkU32)DK_F32_TO_S8_SAT(in.y)) << 8;
        out |= ((DkU32)DK_F32_TO_S8_SAT(in.z)) << 0;
        return out;
    }

    struct DkVec4Hash
    {
        std::size_t operator()(const glm::vec4& vec) const
        {   // Hash combining function
            std::hash<float> floatHasher;
            return floatHasher(vec.x) ^ (floatHasher(vec.y) << 1) ^ (floatHasher(vec.z) << 2) ^ (floatHasher(vec.w) << 3);
        }
    };

}
