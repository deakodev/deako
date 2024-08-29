#pragma once

namespace Deako {

    class RateLimiter
    {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration = std::chrono::seconds;


        RateLimiter(uint32_t counterCycle = 0, uint32_t timeCycle = 1)
            : m_CounterCycle(counterCycle), m_TimeCycle(timeCycle)
        {
            Reset();
        }

        void SetCounterCycle(uint32_t counterCycle) { m_CounterCycle = counterCycle; }
        void SetTimeCycle(uint32_t timeCycle) { m_TimeCycle = Duration(timeCycle); }

        void Reset()
        {
            ResetCounter();
            ResetTime();
        }

        void ResetCounter() { m_Counter = 0; }
        void ResetTime() { m_Time = Clock::now(); m_TimeAtNegativeInfinity = true; }

        void SetCounter(uint32_t counter) { m_Counter = counter; }
        void SetTime(TimePoint time) { m_Time = time; m_TimeAtNegativeInfinity = false; }

        bool Trigger(TimePoint time = TimePoint())
        {
            if (m_CounterCycle == 0 && m_TimeCycle.count() == 0) return true;

            bool trigger = false;
            if (m_CounterCycle > 0) trigger = trigger || ShouldCounterTrigger();
            if (m_TimeCycle.count() > 0)    trigger = trigger || ShouldTimeTrigger(time);

            if (m_CounterCycle > 0) UpdateCounter(trigger);
            if (m_TimeCycle.count() > 0) UpdateTime(trigger, time);

            return trigger;
        }

        bool ShouldCounterTrigger() { return m_Counter == 0; }
        bool ShouldTimeTrigger(TimePoint time)
        {
            if (m_TimeAtNegativeInfinity) return true;

            TimePoint nextTrigger = m_Time + m_TimeCycle;
            return time >= nextTrigger;
        }

        void UpdateCounter(bool triggered)
        {
            if (triggered) m_Counter = 1;  // triggered, set to next state
            else if (++m_Counter >= m_CounterCycle) m_Counter = 0;  // otherwise, just increment and maybe wrap
        }

        void UpdateTime(bool triggered, TimePoint time)
        {
            if (triggered) m_Time = time;
            m_TimeAtNegativeInfinity = false;
        }

    private:
        uint32_t m_CounterCycle;
        uint32_t m_Counter;

        Duration m_TimeCycle;
        TimePoint m_Time;
        bool m_TimeAtNegativeInfinity;
    };

}
