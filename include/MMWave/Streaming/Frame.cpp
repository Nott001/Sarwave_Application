#include "MMWave/Streaming/Frame.hpp"

#include <algorithm>

#include "MMWave/Common/Exceptions.hpp"
#include "MMWave/Streaming/StreamingConstants.hpp"

namespace Sarwave::Core::MMWave::Streaming {

Frame Frame::copyFrom(std::span<const std::uint8_t> source) {
    Frame frame;
    // Never a zero-sized allocation, so even an empty frame reads as "not disposed".
    frame.m_data =
        std::make_unique_for_overwrite<std::uint8_t[]>(std::max<std::size_t>(source.size(), 1));
    std::copy(source.begin(), source.end(), frame.m_data.get());
    frame.m_length = source.size();
    return frame;
}

std::span<const std::uint8_t> Frame::bytes() const {
    if (!m_data) throw ObjectDisposedException("Frame");
    return {m_data.get(), m_length};
}

FrameHeader Frame::header() const {
    return FrameHeader::fromBytes(bytes().subspan(StreamingConstants::MagicWordLength));
}

void Frame::dispose() {
    m_data.reset();
    m_length = 0;
}

}  // namespace Sarwave::Core::MMWave::Streaming
