#pragma once

namespace Deako {

    typedef enum class MouseCode : DkU16
    {
        ButtonLeft = 0,
        ButtonRight = 1,
        ButtonMiddle = 2
    } Mouse;

    inline std::ostream& operator<<(std::ostream& os, MouseCode mouseCode)
    {
        os << static_cast<DkS32>(mouseCode);
        return os;
    }

}
