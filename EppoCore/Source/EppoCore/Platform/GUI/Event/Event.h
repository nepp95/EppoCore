#pragma once

#include <sstream>

namespace Eppo
{
    enum class EventType : uint8_t
    {
        None = 0,

        // Window
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,

        // Keys
        KeyPressed,
        KeyReleased,
        KeyTyped,

        // Mouse
        MouseButtonPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled
    };

    enum EventCategory : uint8_t
    {
        None = 0,
        EventCategoryApplication = 1 << 0,
        EventCategoryInput = 1 << 1,
        EventCategoryKeyboard = 1 << 2,
        EventCategoryMouse = 1 << 3,
        EventCategoryMouseButton = 1 << 4
    };

    #define EVENT_CLASS_TYPE(type)                                                                                                             \
        static EventType GetStaticType() { return EventType::type; }                                                                           \
        virtual EventType GetEventType() const override { return GetStaticType(); }                                                            \
        virtual const char* GetName() const override { return #type; }

    #define EVENT_CLASS_CATEGORY(category)                                                                                                     \
        virtual int GetCategoryFlags() const override { return category; }

    struct Event
    {
        bool Handled = false;

        virtual ~Event() = default;

        [[nodiscard]] virtual EventType GetEventType() const = 0;
        [[nodiscard]] virtual const char* GetName() const = 0;
        [[nodiscard]] virtual int GetCategoryFlags() const = 0;

        [[nodiscard]] virtual std::string ToString() const
        {
            return GetName();
        }

        [[nodiscard]] bool IsInCategory(const EventCategory category) const
        {
            return GetCategoryFlags() & category;
        }

        friend class EventDispatcher;
    };

    class EventDispatcher
    {
    public:
        explicit EventDispatcher(Event& e) : m_Event(e) {}

        template<typename T, typename F>
        bool Dispatch(const F& fn)
        {
            if (m_Event.GetEventType() == T::GetStaticType())
            {
                m_Event.Handled |= fn(static_cast<T&>(m_Event));
                return true;
            }

            return false;
        }

    private:
        Event& m_Event;
    };

    inline std::ostream& operator<<(std::ostream& os, const Event& e) { return os << e.ToString(); }
}
