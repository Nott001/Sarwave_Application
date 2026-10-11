#include "MMWave/Configured/PeopleTracking/DebugLogger.hpp"

#include <format>
#include <string>

#include "MMWave/Common/DotNetFormat.hpp"
#include "MMWave/Configured/PeopleTracking/Tlv/TlvTypes.hpp"
#include "MMWave/Tlv/TlvRange.hpp"

namespace Sarwave::Core::MMWave::Configured::PeopleTracking {
namespace {

using DotNet::formatSingle;
using Tlv::TlvType;

void printValueLabel(std::ostream& out, const std::string& message) {
    out << "      " << message;
}

/// Prints `length` items, `setN` per row; each row starts on a new line with "      | ", and
/// items within a row are separated by `sep`.
template <class Printer>
void printMultiple(std::ostream& out, int length, int setN, const char* sep, Printer&& printer) {
    for (int i = 0; i < length; i++) {
        const int j = i % setN;
        if (j == 0) out << "\n      | ";
        printer(i);
        if (setN > 1 && i < length - 1) out << sep;
    }
}

template <class Printer>
void printMultiple(std::ostream& out, int length, Printer&& printer) {
    printMultiple(out, length, 1, "", std::forward<Printer>(printer));
}

}  // namespace

void DebugLogger::printFrameDataVerbose(const Streaming::Frame& frame, TimeSpan elapsed) {
    std::ostream& out = *m_out;

    out << "New Frame (#" << m_counter << ") (" << DotNet::formatMinutesSecondsMicros(elapsed)
        << ")\n";
    m_counter++;

    for (const MMWave::Tlv::TlvEntry& tlv : MMWave::Tlv::TlvRange(frame)) {
        out << "   - Tlv Type: " << Tlv::toString(static_cast<TlvType>(tlv.type))
            << " (uint=" << tlv.type << ")\n";

        if (!conf.hideBytes) {
            printValueLabel(out, "Bytes (" + std::to_string(tlv.bytes.size()) + ")");
            printMultiple(out, static_cast<int>(tlv.bytes.size()), 16, " ", [&](int i) {
                out << std::format("{:02x}", tlv.bytes[static_cast<std::size_t>(i)]);
            });
            out << "\n";
        }

        switch (static_cast<TlvType>(tlv.type)) {
            case TlvType::TlvTargetList: {
                const auto targetRange = Tlv::TlvContentRange<Tlv::TlvTargetList>::convert(tlv);
                printValueLabel(out,
                                "Targets (Count=" + std::to_string(targetRange.count()) + "):");
                printMultiple(out, targetRange.count(), [&](int i) {
                    const auto target = targetRange[i];
                    out << "      id:" << target.targetId.toString()
                        << " x:" << formatSingle(target.posX) << " y:" << formatSingle(target.posY)
                        << " z:" << formatSingle(target.posZ);
                });
                break;
            }
            case TlvType::TlvTargetIndex: {
                const auto indexRange = Tlv::TlvContentRange<Tlv::TlvTargetIndex>::convert(tlv);
                printValueLabel(out, "Indexes (Count=" + std::to_string(indexRange.count()) + "):");
                printMultiple(out, indexRange.count(), 10, " ",
                              [&](int i) { out << std::format("{:>3}", indexRange[i].index); });
                break;
            }
            case TlvType::TlvTargetHeight: {
                const auto heightRange = Tlv::TlvContentRange<Tlv::TlvTargetHeight>::convert(tlv);
                printValueLabel(out,
                                "Heights (Count=" + std::to_string(heightRange.count()) + "):");
                printMultiple(out, heightRange.count(), [&](int i) {
                    const auto height = heightRange[i];
                    out << "      id:" << height.targetId.toString()
                        << " height:" << formatSingle(height.minZ) << "~"
                        << formatSingle(height.maxZ);
                });
                break;
            }
            case TlvType::TlvPointCloud: {
                const auto pointClouds = Tlv::TlvPointCloud::decompile(tlv);
                printValueLabel(
                    out, "Points (Count=" + std::to_string(pointClouds.points.count()) + "):");
                break;
            }
            case TlvType::TlvPresenceIndication: {
                const bool presence = Tlv::TlvPresenceIndication::presence(tlv);
                printValueLabel(out, std::string("Presence: ") + (presence ? "true" : "false"));
                break;
            }
        }

        out << "\n";
    }
    out.flush();
}

}  // namespace Sarwave::Core::MMWave::Configured::PeopleTracking
