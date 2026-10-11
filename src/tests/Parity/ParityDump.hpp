#pragma once

// The text dumps shared by CoreParityHarness and the parity unit tests. The C# twin
// (README.md) prints exactly the same layout from the original Sarwave.Core.MMWave.

#include <algorithm>
#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Configured/PeopleTracking/Association/CombinedManager.hpp"
#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"
#include "MMWave/Recording/FrameRecording.hpp"

namespace SarwaveParity {

using namespace Sarwave::Core;
using namespace Sarwave::Core::MMWave;
using namespace Sarwave::Core::MMWave::Configured::PeopleTracking;

inline std::string F(float v) {
    return DotNet::formatSingle(v);
}

inline std::string V(const Vector3& v) {
    return F(v.x) + "," + F(v.y) + "," + F(v.z);
}

inline std::string Ex(const std::exception& e) {
    return "EXCEPTION " + dotNetTypeName(e) + ": " + e.what();
}

inline std::string joinSorted(std::vector<std::string> items, const char* sep) {
    std::sort(items.begin(), items.end());
    std::string out;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i) out += sep;
        out += items[i];
    }
    return out;
}

/// DebugLogger output for every frame of a recording (stops at the first exception, printing it).
inline void dumpLog(std::ostream& out, const Recording::FrameRecording& rec, bool hideBytes) {
    DebugLogger logger(DebugLogger::Config{hideBytes}, out);
    for (int i = 0; i < rec.count(); i++) {
        try {
            logger.printFrameDataVerbose(rec.frames()[static_cast<std::size_t>(i)],
                                         rec.timestamps()[static_cast<std::size_t>(i)]);
        } catch (const std::exception& e) {
            out << Ex(e) << "\n";
            break;
        }
    }
}

/// CombinedManager state after every frame of a recording.
inline void dumpManager(std::ostream& out, const Recording::FrameRecording& rec,
                        double durationMs) {
    Association::CombinedManager manager(TimeSpans::fromSeconds(durationMs / 1000.0));
    std::ostringstream sb;
    for (int i = 0; i < rec.count(); i++) {
        try {
            manager.update(rec.frames()[static_cast<std::size_t>(i)],
                           rec.timestamps()[static_cast<std::size_t>(i)]);
        } catch (const std::exception& e) {
            sb << Ex(e) << "\n";
            break;
        }
        sb << "F " << i << " presence=" << (manager.presence() ? "True" : "False") << "\n";
        for (const auto* t : manager.allTargetObjects()) {
            sb << " T " << t->objectId() << " "
               << (t->state() == Association::TargetObject::TargetState::Active ? "Active"
                                                                                : "Dropped")
               << " pos=" << V(t->pos) << " vel=" << V(t->vel) << " acc=" << V(t->acc)
               << " z=" << F(t->minZ) << "~" << F(t->maxZ) << " g=" << F(t->g)
               << " c=" << F(t->confidenceLevel) << " ec=" << F(t->ec.m[0][0]) << ","
               << F(t->ec.m[1][2]) << "," << F(t->ec.m[3][3]) << "\n";
        }
        for (const auto& e : manager.pointClouds()) {
            sb << " C " << e->timestamp().count()
               << " assoc=" << (e->isAssociated() ? "True" : "False")
               << " n=" << e->pointValues().size() << "\n";
            if (e->isAssociated()) {
                for (int id : e->objectIdsUsed(false)) {
                    const auto pts = e->getPointsOf(id);
                    std::vector<std::string> items;
                    for (const auto& p : pts) {
                        items.push_back(F(p.cartesian.x) + "," + F(p.cartesian.y) + "," +
                                        F(p.cartesian.z) + "," + F(p.doppler) + "," + F(p.snr) +
                                        "," + F(p.radial.range));
                    }
                    sb << "  G " << id << " " << pts.size() << ": " << joinSorted(items, ";")
                       << "\n";
                }
                std::string used;
                const auto ids = e->objectIdsUsed(true);
                for (std::size_t k = 0; k < ids.size(); ++k)
                    used += (k ? "," : "") + std::to_string(ids[k]);
                sb << "  X " << used << "\n";
            } else {
                std::vector<std::string> items;
                for (const auto& p : e->pointValues()) {
                    items.push_back(F(p.cartesian.x) + "," + F(p.cartesian.y) + "," +
                                    F(p.cartesian.z) + "," + F(p.doppler) + "," + F(p.snr));
                }
                // Unassociated points keep their wire order (the C# harness doesn't sort these).
                std::string joined;
                for (std::size_t k = 0; k < items.size(); ++k) joined += (k ? ";" : "") + items[k];
                sb << "  U " << joined << "\n";
            }
        }
    }
    out << sb.str();
}

}  // namespace SarwaveParity
