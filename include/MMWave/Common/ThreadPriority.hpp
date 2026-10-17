#pragma once

#include <thread>

#if defined(__linux__)
#include <pthread.h>
#include <sched.h>
#endif

namespace Sarwave::Core::MMWave {

/// Asks Linux to run `thread` under the real-time FIFO policy at `priority` (1 to 99). It then
/// runs the moment it is ready, ahead of every normal thread: the GUI, the desktop, other
/// programs. Meant for threads that sleep most of the time and must react quickly when woken,
/// such as the one reading the data port.
///
/// Needs permission: an rtprio limit of at least `priority` (/etc/security/limits.d) or
/// CAP_SYS_NICE. Returns false and leaves the thread as it was without it, and on other systems.
inline bool trySetRealtimePriority(std::thread& thread, int priority) {
#if defined(__linux__)
    sched_param param{};
    param.sched_priority = priority;
    return pthread_setschedparam(thread.native_handle(), SCHED_FIFO, &param) == 0;
#else
    (void)thread;
    (void)priority;
    return false;
#endif
}

}  // namespace Sarwave::Core::MMWave
