#pragma once

#include <iostream>
#include <ostream>

#include "MMWave/Common/TimeSpan.hpp"
#include "MMWave/Streaming/Frame.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking {

/// Prints a People Tracking frame's TLVs to the console, one block per frame, in exactly the
/// layout of the C# DebugLogger.
class DebugLogger {
   public:
    struct Config {
        bool hideBytes = false;
    };

    DebugLogger() : DebugLogger(Config{false}) {
    }

    explicit DebugLogger(Config conf, std::ostream& out = std::cout) : conf(conf), m_out(&out) {
    }

    Config conf;

    void printFrameDataVerbose(const Streaming::Frame& frame, TimeSpan elapsed);

   private:
    int m_counter = 0;
    std::ostream* m_out;
};

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking
