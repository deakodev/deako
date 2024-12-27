#pragma once

#include "deako_core.h"
#include "deako_input.h"

#include <glm/glm.hpp>

namespace Deako {

	enum class EventType
	{
		None = 0,
		WindowClose, WindowMinimized, WindowRestored, WindowResize,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	enum EventCategory
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput = BIT(1),
		EventCategoryKeyboard = BIT(2),
		EventCategoryMouse = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

	class Event
	{
	public:
		virtual ~Event() = default;

		virtual EventType GetEventType() const = 0;
		virtual int GetCategoryFlags() const = 0;

		bool IsInCategory(EventCategory category) const { return GetCategoryFlags() & category; }

		bool Handled = false;
	};

	class WindowCloseEvent : public Event
	{
	public:
		static EventType GetStaticType() { return EventType::WindowClose; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
	};

	class WindowMinimizedEvent : public Event
	{
	public:
		static EventType GetStaticType() { return EventType::WindowMinimized; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
	};

	class WindowRestoredEvent : public Event
	{
	public:
		static EventType GetStaticType() { return EventType::WindowRestored; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryApplication; }
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(int width, int height) : Width(width), Height(height) {}

		static EventType GetStaticType() { return EventType::WindowResize; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryApplication; }

	private:
		int Width, Height;
	};

	class KeyEvent : public Event
	{
	public:
		KeyCode GetKeyCode() const { return m_KeyCode; }
		virtual int GetCategoryFlags() const override { return EventCategoryKeyboard | EventCategoryInput; }

	protected:
		KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}

	private:
		KeyCode m_KeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(KeyCode keycode, bool isRepeat = false) : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

		bool IsRepeat() const { return m_IsRepeat; }

		static EventType GetStaticType() { return EventType::WindowResize; }
		virtual EventType GetEventType() const override { return GetStaticType(); }

	private:
		bool m_IsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		static EventType GetStaticType() { return EventType::WindowResize; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		static EventType GetStaticType() { return EventType::WindowResize; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
	};

	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const glm::vec2& position) : m_Position(position) {}

		const glm::vec2& GetMousePosition() const { return m_Position; }

		static EventType GetStaticType() { return EventType::MouseMoved; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton; }

	private:
		glm::vec2 m_Position;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const glm::vec2& offset) : m_Offset(offset) {}

		const glm::vec2& GetMouseScrollOffset() const { return m_Offset; }

		static EventType GetStaticType() { return EventType::MouseScrolled; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
		virtual int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput; }

	private:
		glm::vec2 m_Offset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		MouseCode GetMouseButton() const { return m_Button; }

		virtual int GetCategoryFlags() const override { return EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton; }

	protected:
		MouseButtonEvent(MouseCode button) : m_Button(button) {}

	private:
		MouseCode m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(MouseCode button) : MouseButtonEvent(button) {}

		static EventType GetStaticType() { return EventType::MouseButtonPressed; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(MouseCode button) : MouseButtonEvent(button) {}

		static EventType GetStaticType() { return EventType::MouseButtonReleased; }
		virtual EventType GetEventType() const override { return GetStaticType(); }
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event) : m_Event(event) {}

		template<typename T, typename F>
		bool Dispatch(const F& eventHandler)
		{
			if (m_Event.GetEventType() == T::GetStaticType())
			{
				m_Event.Handled |= eventHandler(static_cast<T&>(m_Event));
				return true;
			}
			return false;
		}

	private:
		Event& m_Event;
	};

}
