#pragma once

#include "Deako/Event/Event.h"
#include "Deako/Event/MouseCodes.h"

namespace Deako {

    class MouseMovedEvent : public Event
    {
    public:
        MouseMovedEvent(const DkF32 x, const DkF32 y)
            : m_MouseX(x), m_MouseY(y)
        {
        }

        DkF32 GetX() const { return m_MouseX; }
        DkF32 GetY() const { return m_MouseY; }

        EVENT_CLASS_TYPE(MouseMoved)
            EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

    private:
        DkF32 m_MouseX;
        DkF32 m_MouseY;
    };

    class MouseScrolledEvent : public Event
    {
    public:
        MouseScrolledEvent(const DkF32 xOffset, const DkF32 yOffset)
            : m_XOffset(xOffset), m_YOffset(yOffset)
        {
        }

        DkF32 GetXOffset() const { return m_XOffset; }
        DkF32 GetYOffset() const { return m_YOffset; }

        EVENT_CLASS_TYPE(MouseScrolled)
            EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

    private:
        DkF32 m_XOffset;
        DkF32 m_YOffset;
    };

    class MouseButtonEvent : public Event
    {
    public:
        MouseCode GetMouseButton() const { return m_Button; }

        EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

    protected:
        MouseButtonEvent(const MouseCode button)
            : m_Button(button)
        {
        }

        MouseCode m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonPressedEvent(const MouseCode button)
            : MouseButtonEvent(button)
        {
        }

        EVENT_CLASS_TYPE(MouseButtonPressed)
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent
    {
    public:
        MouseButtonReleasedEvent(const MouseCode button)
            : MouseButtonEvent(button)
        {
        }

        EVENT_CLASS_TYPE(MouseButtonReleased)
    };

}
