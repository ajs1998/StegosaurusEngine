#pragma once

#include "Core.h"

namespace Steg {

    class StegTimer {

    public:

        enum TimerLabel {
            ENCODE,
            ENCRYPT,
            DECODE,
            DECRYPT,
            TOTAL // This one has to be last in the list
        };

        StegTimer() = delete;

        static void StartTimer(TimerLabel timer);

        static void EndTimer(TimerLabel timer);

        static void PrintTimers();

    private:

        static std::array<std::chrono::steady_clock::time_point, TimerLabel::TOTAL + 1> timers;

        static std::array<std::chrono::milliseconds, TimerLabel::TOTAL + 1> elapsed;

        static std::string GetTimerName(TimerLabel timer);

    };

}
