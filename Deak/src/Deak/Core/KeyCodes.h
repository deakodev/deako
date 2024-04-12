#pragma once

namespace Deak
{
    typedef enum class KeyCode : uint16_t
    {
        // From glfw3.h
        Space = 32,
        Apostrophe = 39, /* ' */
        Comma = 44, /* , */
        Minus = 45, /* - */
        Period = 46, /* . */
        Slash = 47, /* / */

        D0 = 48, /* 0 */
        D1 = 49, /* 1 */
        D2 = 50, /* 2 */
        D3 = 51, /* 3 */
        D4 = 52, /* 4 */
        D5 = 53, /* 5 */
        D6 = 54, /* 6 */
        D7 = 55, /* 7 */
        D8 = 56, /* 8 */
        D9 = 57, /* 9 */

        Semicolon = 59, /* ; */
        Equal = 61, /* = */

        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,

        LeftBracket = 91,  /* [ */
        Backslash = 92,  /* \ */
        RightBracket = 93,  /* ] */
        GraveAccent = 96,  /* ` */

        World1 = 161, /* non-US #1 */
        World2 = 162, /* non-US #2 */

        /* Function keys */
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,

        /* Keypad */
        KP0 = 320,
        KP1 = 321,
        KP2 = 322,
        KP3 = 323,
        KP4 = 324,
        KP5 = 325,
        KP6 = 326,
        KP7 = 327,
        KP8 = 328,
        KP9 = 329,
        KPDecimal = 330,
        KPDivide = 331,
        KPMultiply = 332,
        KPSubtract = 333,
        KPAdd = 334,
        KPEnter = 335,
        KPEqual = 336,

        LeftShift = 340,
        LeftControl = 341,
        LeftAlt = 342,
        LeftSuper = 343,
        RightShift = 344,
        RightControl = 345,
        RightAlt = 346,
        RightSuper = 347,
        Menu = 348
    } Key;

    inline std::ostream& operator<<(std::ostream& os, KeyCode keyCode)
    {
        os << static_cast<int32_t>(keyCode);
        return os;
    }
}

// From glfw3.h
#define DK_KEY_SPACE           ::Deak::Key::Space
#define DK_KEY_APOSTROPHE      ::Deak::Key::Apostrophe    /* ' */
#define DK_KEY_COMMA           ::Deak::Key::Comma         /* , */
#define DK_KEY_MINUS           ::Deak::Key::Minus         /* - */
#define DK_KEY_PERIOD          ::Deak::Key::Period        /* . */
#define DK_KEY_SLASH           ::Deak::Key::Slash         /* / */
#define DK_KEY_0               ::Deak::Key::D0
#define DK_KEY_1               ::Deak::Key::D1
#define DK_KEY_2               ::Deak::Key::D2
#define DK_KEY_3               ::Deak::Key::D3
#define DK_KEY_4               ::Deak::Key::D4
#define DK_KEY_5               ::Deak::Key::D5
#define DK_KEY_6               ::Deak::Key::D6
#define DK_KEY_7               ::Deak::Key::D7
#define DK_KEY_8               ::Deak::Key::D8
#define DK_KEY_9               ::Deak::Key::D9
#define DK_KEY_SEMICOLON       ::Deak::Key::Semicolon     /* ; */
#define DK_KEY_EQUAL           ::Deak::Key::Equal         /* = */
#define DK_KEY_A               ::Deak::Key::A
#define DK_KEY_B               ::Deak::Key::B
#define DK_KEY_C               ::Deak::Key::C
#define DK_KEY_D               ::Deak::Key::D
#define DK_KEY_E               ::Deak::Key::E
#define DK_KEY_F               ::Deak::Key::F
#define DK_KEY_G               ::Deak::Key::G
#define DK_KEY_H               ::Deak::Key::H
#define DK_KEY_I               ::Deak::Key::I
#define DK_KEY_J               ::Deak::Key::J
#define DK_KEY_K               ::Deak::Key::K
#define DK_KEY_L               ::Deak::Key::L
#define DK_KEY_M               ::Deak::Key::M
#define DK_KEY_N               ::Deak::Key::N
#define DK_KEY_O               ::Deak::Key::O
#define DK_KEY_P               ::Deak::Key::P
#define DK_KEY_Q               ::Deak::Key::Q
#define DK_KEY_R               ::Deak::Key::R
#define DK_KEY_S               ::Deak::Key::S
#define DK_KEY_T               ::Deak::Key::T
#define DK_KEY_U               ::Deak::Key::U
#define DK_KEY_V               ::Deak::Key::V
#define DK_KEY_W               ::Deak::Key::W
#define DK_KEY_X               ::Deak::Key::X
#define DK_KEY_Y               ::Deak::Key::Y
#define DK_KEY_Z               ::Deak::Key::Z
#define DK_KEY_LEFT_BRACKET    ::Deak::Key::LeftBracket   /* [ */
#define DK_KEY_BACKSLASH       ::Deak::Key::Backslash     /* \ */
#define DK_KEY_RIGHT_BRACKET   ::Deak::Key::RightBracket  /* ] */
#define DK_KEY_GRAVE_ACCENT    ::Deak::Key::GraveAccent   /* ` */
#define DK_KEY_WORLD_1         ::Deak::Key::World1        /* non-US #1 */
#define DK_KEY_WORLD_2         ::Deak::Key::World2        /* non-US #2 */

/* Function keys */
#define DK_KEY_ESCAPE          ::Deak::Key::Escape
#define DK_KEY_ENTER           ::Deak::Key::Enter
#define DK_KEY_TAB             ::Deak::Key::Tab
#define DK_KEY_BACKSPACE       ::Deak::Key::Backspace
#define DK_KEY_INSERT          ::Deak::Key::Insert
#define DK_KEY_DELETE          ::Deak::Key::Delete
#define DK_KEY_RIGHT           ::Deak::Key::Right
#define DK_KEY_LEFT            ::Deak::Key::Left
#define DK_KEY_DOWN            ::Deak::Key::Down
#define DK_KEY_UP              ::Deak::Key::Up
#define DK_KEY_PAGE_UP         ::Deak::Key::PageUp
#define DK_KEY_PAGE_DOWN       ::Deak::Key::PageDown
#define DK_KEY_HOME            ::Deak::Key::Home
#define DK_KEY_END             ::Deak::Key::End
#define DK_KEY_CAPS_LOCK       ::Deak::Key::CapsLock
#define DK_KEY_SCROLL_LOCK     ::Deak::Key::ScrollLock
#define DK_KEY_NUM_LOCK        ::Deak::Key::NumLock
#define DK_KEY_PRINT_SCREEN    ::Deak::Key::PrintScreen
#define DK_KEY_PAUSE           ::Deak::Key::Pause
#define DK_KEY_F1              ::Deak::Key::F1
#define DK_KEY_F2              ::Deak::Key::F2
#define DK_KEY_F3              ::Deak::Key::F3
#define DK_KEY_F4              ::Deak::Key::F4
#define DK_KEY_F5              ::Deak::Key::F5
#define DK_KEY_F6              ::Deak::Key::F6
#define DK_KEY_F7              ::Deak::Key::F7
#define DK_KEY_F8              ::Deak::Key::F8
#define DK_KEY_F9              ::Deak::Key::F9
#define DK_KEY_F10             ::Deak::Key::F10
#define DK_KEY_F11             ::Deak::Key::F11
#define DK_KEY_F12             ::Deak::Key::F12
#define DK_KEY_F13             ::Deak::Key::F13
#define DK_KEY_F14             ::Deak::Key::F14
#define DK_KEY_F15             ::Deak::Key::F15
#define DK_KEY_F16             ::Deak::Key::F16
#define DK_KEY_F17             ::Deak::Key::F17
#define DK_KEY_F18             ::Deak::Key::F18
#define DK_KEY_F19             ::Deak::Key::F19
#define DK_KEY_F20             ::Deak::Key::F20
#define DK_KEY_F21             ::Deak::Key::F21
#define DK_KEY_F22             ::Deak::Key::F22
#define DK_KEY_F23             ::Deak::Key::F23
#define DK_KEY_F24             ::Deak::Key::F24
#define DK_KEY_F25             ::Deak::Key::F25

/* Keypad */
#define DK_KEY_KP_0            ::Deak::Key::KP0
#define DK_KEY_KP_1            ::Deak::Key::KP1
#define DK_KEY_KP_2            ::Deak::Key::KP2
#define DK_KEY_KP_3            ::Deak::Key::KP3
#define DK_KEY_KP_4            ::Deak::Key::KP4
#define DK_KEY_KP_5            ::Deak::Key::KP5
#define DK_KEY_KP_6            ::Deak::Key::KP6
#define DK_KEY_KP_7            ::Deak::Key::KP7
#define DK_KEY_KP_8            ::Deak::Key::KP8
#define DK_KEY_KP_9            ::Deak::Key::KP9
#define DK_KEY_KP_DECIMAL      ::Deak::Key::KPDecimal
#define DK_KEY_KP_DIVIDE       ::Deak::Key::KPDivide
#define DK_KEY_KP_MULTIPLY     ::Deak::Key::KPMultiply
#define DK_KEY_KP_SUBTRACT     ::Deak::Key::KPSubtract
#define DK_KEY_KP_ADD          ::Deak::Key::KPAdd
#define DK_KEY_KP_ENTER        ::Deak::Key::KPEnter
#define DK_KEY_KP_EQUAL        ::Deak::Key::KPEqual

#define DK_KEY_LEFT_SHIFT      ::Deak::Key::LeftShift
#define DK_KEY_LEFT_CONTROL    ::Deak::Key::LeftControl
#define DK_KEY_LEFT_ALT        ::Deak::Key::LeftAlt
#define DK_KEY_LEFT_SUPER      ::Deak::Key::LeftSuper
#define DK_KEY_RIGHT_SHIFT     ::Deak::Key::RightShift
#define DK_KEY_RIGHT_CONTROL   ::Deak::Key::RightControl
#define DK_KEY_RIGHT_ALT       ::Deak::Key::RightAlt
#define DK_KEY_RIGHT_SUPER     ::Deak::Key::RightSuper
#define DK_KEY_MENU            ::Deak::Key::Menu
