#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <span>
#include <type_traits>

#include "MMWave/Tlv/TlvEntry.hpp"

namespace Sarwave::Core::MMWave::Tlv {

/// A TLV payload seen as an array of fixed-size records. Count is the payload length divided by
/// sizeof(T) (a trailing partial record is ignored). Records are copied out with memcpy, so the
/// payload needs no particular alignment. Only valid while the owning Frame is.
template <class T>
    requires std::is_trivially_copyable_v<T>
class TlvContentRange {
   public:
    TlvContentRange() = default;

    explicit TlvContentRange(std::span<const std::uint8_t> bytes) : m_data(bytes) {
    }

    explicit TlvContentRange(const TlvEntry& entry) : m_data(entry.bytes) {
    }

    static TlvContentRange convert(const TlvEntry& entry) {
        return TlvContentRange(entry);
    }

    [[nodiscard]] int count() const {
        return static_cast<int>(m_data.size() / sizeof(T));
    }

    /// Bounds-checked, like indexing the C# span: throws ArgumentOutOfRangeException.
    [[nodiscard]] T operator[](int i) const;

    [[nodiscard]] std::span<const std::uint8_t> bytes() const {
        return m_data;
    }

    class Iterator {
       public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;

        Iterator() = default;
        Iterator(const std::uint8_t* at) : m_at(at) {
        }

        T operator*() const {
            T value;
            std::memcpy(&value, m_at, sizeof(T));
            return value;
        }

        Iterator& operator++() {
            m_at += sizeof(T);
            return *this;
        }

        Iterator operator++(int) {
            Iterator copy = *this;
            ++*this;
            return copy;
        }

        friend bool operator==(const Iterator&, const Iterator&) = default;

       private:
        const std::uint8_t* m_at = nullptr;
    };

    [[nodiscard]] Iterator begin() const {
        return {m_data.data()};
    }

    [[nodiscard]] Iterator end() const {
        return {m_data.data() + static_cast<std::size_t>(count()) * sizeof(T)};
    }

   private:
    std::span<const std::uint8_t> m_data;
};

}  // namespace Sarwave::Core::MMWave::Tlv

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::MMWave::Tlv {

template <class T>
    requires std::is_trivially_copyable_v<T>
T TlvContentRange<T>::operator[](int i) const {
    if (i < 0 || i >= count()) throw ArgumentOutOfRangeException("index");
    T value;
    std::memcpy(&value, m_data.data() + static_cast<std::size_t>(i) * sizeof(T), sizeof(T));
    return value;
}

}  // namespace Sarwave::Core::MMWave::Tlv
