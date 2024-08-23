#pragma once 

namespace Deako {

    enum class ImageFormat
    {
        None = 0,
        DK_R8G8B8A8_UNORM,
        DK_R16G16B16A16_SFLOAT,
    };

    enum class ImageUsage
    {
        None = 0,
        DK_SAMPLED_BIT,
    };

    enum class ImageLayout
    {
        None = 0,
        DK_SHADER_READ_ONLY_OPTIMAL,
    };

}
