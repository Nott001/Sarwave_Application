#pragma once

#include <chrono>

#include "MMWave/Porter.hpp"
#include "MMWave/ThreadedStreaming.hpp"

namespace MMWave::Configured::PeopleTracking {
void onFrameProcessed(const Streaming::Frame& frame,
                             const std::chrono::steady_clock::time_point& start);
}