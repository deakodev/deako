#pragma once

#include "dkpch.h"
#include "Deako/Events/Event.h"

namespace Deako {

    class WindowResizeEvent : public Event
    {
    public:
        WindowResizeEvent(unsigned int width, unsigned int height)
            : m_Width(width), m_Height(height)
        {
        }

        unsigned int GetWidth() const { return m_Width; }
        unsigned int GetHeight() const { return m_Height; }

        EVENT_CLASS_TYPE(WindowResize)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

    private:
        unsigned int m_Width{};
        unsigned int m_Height{};
    };

    class WindowMinimizedEvent : public Event
    {
    public:
        WindowMinimizedEvent() = default;

        EVENT_CLASS_TYPE(WindowMinimized)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

    };

    class WindowRestoredEvent : public Event
    {
    public:
        WindowRestoredEvent() = default;

        EVENT_CLASS_TYPE(WindowRestored)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)

    };
    class WindowCloseEvent : public Event
    {
    public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
            EVENT_CLASS_CATEGORY(EventCategoryWindow)
    };

}
