/*
    Copyright (c) 2018-2020 Piotr Doan. All rights reserved.
*/

#pragma once

#include <limits>

/*
    System Timer

    Keeps track of time and provides utilities such as automatic
    calculation of delta time between ticks and frame rate measurement.

    void ExampleSystemTimer()
    {
        // Create a timer instance.
        System::Timer timer;
        timer.Initialize();

        // Run a loop that will measure delta time.
        while(true)
        {
            // Calculate delta time between two last ticks.
            float dt = timer.CalculateTickDelta();
            Log() << "Current delta time: " << dt;

            // Perform some calculations over a frame and tick the timer.
            Sleep(1000);
            timer.Tick();
        }
    }
*/

namespace System
{
    // Timer class.
    class Timer : private NonCopyable
    {
    public:
        // Constant values.
        static constexpr float MaximumFloat = std::numeric_limits<float>::max();

    public:
        Timer();
        virtual ~Timer();

        // Move constructor and operator.
        Timer(Timer&& other);
        Timer& operator=(Timer&& other);

        // Initializes the timer instance.
        virtual bool Initialize();

        // Resets the timer.
        virtual void Reset();

        // Ticks the timer to match the current system time.
        void Tick(float maximumDelta = 0.0f);

        // Ticks the timer to match the current time of another timer.
        void Tick(const Timer& timer);

    public:
        // Gets the delta time in ticks between last two ticks.
        uint64_t GetDeltaTicks() const;

        // Gets the delta time in seconds between last two ticks.
        float GetDeltaTime() const;

        // Gets the current time in seconds.
        double GetSystemTime() const;

    protected:
        // Time tracking values.
        uint64_t m_timerFrequency;
        uint64_t m_currentTimeCounter;
        uint64_t m_previousTimeCounter;
    };
}
