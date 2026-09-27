#pragma once

#include <chrono>
#include <format>

#include <ostream>
#include <print>
#include <string>

#include "TlvOutput.hpp"
#include "TlvTypes.hpp"
#include "MMWave/Porter.hpp"
#include "MMWave/ThreadedStreaming.hpp"
#include "MMWave/TlvCore.hpp"
#include "Debug.hpp"

namespace MMWave::Configured::PeopleTracking {
namespace {
    template <typename... Args>
    void printValueLabel(std::format_string<Args...> fmt, Args && ... args) {
        std::print("      ");
        std::vprint_unicode(fmt.get(), std::make_format_args(args...));
    }

    using IndexPrinter = std::function<void(uint32_t index)>;
    void printMultiple(const uint32_t length, const uint32_t set_n, const std::string& sep,
                       const IndexPrinter& printer) {
        uint32_t i = 0;
        for (i = 0; i < length; i++) {
            const uint32_t j = i % set_n;
            if (j == 0) {
                std::print("\n      | ");
            }
            printer(i);
            if (set_n > 1 && i < length - 1) std::print("{}", sep);
        }
    }

    void printMultiple(const uint32_t length, const IndexPrinter& printer) {
        printMultiple(length, 1, "", printer);
    }
}

void onFrameProcessed(const Streaming::Frame& frame,
                             const std::chrono::steady_clock::time_point& start) {
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                       std::chrono::steady_clock::now() - start)
                       .count();
    std::println("New Frame ({} us)", elapsed);
    for (const auto& tlv : TlvRange(frame)) {
        std::println("   - Tlv Type: {} (uint={})", tlv.type, static_cast<uint32_t>(tlv.type));
        printValueLabel("Bytes ({})", tlv.length);
        printMultiple(tlv.length, 16, " ",
                      [&tlv](const int i) { std::print("{:02x}", tlv.payload[i]); });
        std::print("\n");

        switch (tlv.type) {
            case TLV_TARGET_LIST: {
                auto tracked_target = TargetList::range(tlv);
                printValueLabel("Targets (Count={}):", tracked_target.getCount());
                printMultiple(tracked_target.getCount(), [&tracked_target](const int i) {
                    const auto target = tracked_target[i];
                    std::print("      id:{} x:{} y:{} z:{}", target.targetID, target.posX,
                               target.posY, target.posZ);
                });
                break;
            }
            case TLV_TARGET_INDEX: {
                auto target_list = TargetIndex::range(tlv);
                printValueLabel("Indexes (Count={}):", target_list.getCount());
                printMultiple(target_list.getCount(), 10, " ", [&target_list](const int i) {
                    const auto index = target_list[i];
                    std::print("{:3}", index);
                });
                break;
            }
            case TLV_TARGET_HEIGHT: {
                auto target_height = TargetHeight::range(tlv);
                printValueLabel("Heights (Count={}):", target_height.getCount());
                printMultiple(target_height.getCount(), [&target_height](const int i) {
                    const auto& [targetID, maxZ, minZ] = target_height[i];
                    std::print("      id:{} height:{}~{}", targetID, minZ, maxZ);
                });
                break;
            }
            case TLV_POINT_CLOUD: {
                auto point_clouds = PointCloud::range(tlv);
                printValueLabel("Points (Count={}):", point_clouds.getCount());
                break;
            }
            case TLV_PRESENCE_INDICATION: {
                const bool presence = PresenceIndication::presence(tlv);
                printValueLabel("Presence: {}", presence ? "true" : "false");
                break;
            }
            default:;
        }

        std::print("\n");
    }
}
}  // MMWave::Configured::PeopleTracking