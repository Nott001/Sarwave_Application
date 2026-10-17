#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>

#include "Imu/Device/Icm42688Settings.hpp"
#include "Imu/ImuSample.hpp"

namespace Sarwave::Core::Imu::Device {

/// One FIFO packet, converted to physical units. Axes are the IMU's own, as printed on the chip.
struct Icm42688Reading {
    /// Specific force in g: reads +1 g along whichever axis points up at rest.
    Vector3 acceleration;
    /// In °/s, right-handed about each axis.
    Vector3 angularRate;
    /// Die temperature in °C (FIFO resolution, about 0.5 °C).
    float temperature = 0;
    /// The chip's 16-bit ODR timestamp. Only differences between packets mean anything, and its
    /// tick is not 1 µs despite the datasheet: it runs at 15/16 of that relative to the output
    /// data rate, and drifts with the chip's own oscillator. See FifoClock for how it is put on
    /// the host clock.
    std::uint16_t timestamp = 0;
};

/// TDK InvenSense ICM-42688-P six-axis IMU on Linux SPI (spidev), read through its FIFO.
///
/// The FIFO runs in stream mode with the accelerometer, gyroscope, temperature and a timestamp
/// in every 16-byte packet, so samples are taken by the chip at an exact rate and the host only
/// has to empty it every few milliseconds. 2 KB holds 128 packets: 640 ms at 200 Hz, far longer
/// than any scheduling hiccup on the Raspberry Pi.
///
/// Not thread-safe: one thread at a time. The buffers are allocated once, so reading allocates
/// nothing.
class Icm42688 {
   public:
    static constexpr std::uint8_t ExpectedWhoAmI = 0x47;
    static constexpr int FifoPacketSize = 16;
    static constexpr int FifoSize = 2048;
    static constexpr int MaxPacketsPerRead = FifoSize / FifoPacketSize;

    /// Opens the SPI device, checks that an ICM-42688-P answers, resets it, configures it and
    /// starts its sensors. Returns once the gyroscope has started and the FIFO is empty.
    ///
    /// Throws ImuDeviceException when nothing answered as an ICM-42688-P or it would not keep
    /// its configuration, IOException when the SPI device could not be opened (SPI disabled, no
    /// permission -- join the "spi" group -- or not Linux).
    static std::unique_ptr<Icm42688> open(const Icm42688Settings& settings = {});

    ~Icm42688();
    Icm42688(const Icm42688&) = delete;
    Icm42688& operator=(const Icm42688&) = delete;

    [[nodiscard]] const Icm42688Settings& settings() const {
        return m_settings;
    }

    std::uint8_t readWhoAmI();

    /// Moves every complete packet waiting in the FIFO into `destination`, oldest first. Packets
    /// the chip marks invalid (sensor still starting) are dropped.
    ///
    /// overflowed: the FIFO had filled up since the last call, so samples were lost and the
    /// packets left in it no longer follow on from the last ones read. The FIFO is flushed and
    /// nothing is returned.
    /// Returns the number of readings written.
    int readFifo(std::span<Icm42688Reading> destination, bool& overflowed);

    /// Empties the FIFO.
    void flushFifo();

    /// Turns the sensors off and closes the SPI device.
    void dispose();

   private:
    Icm42688(int fd, const Icm42688Settings& settings);

    void initialize();
    void requireWhoAmI();
    bool tryDecode(std::span<const std::uint8_t> packet, Icm42688Reading& reading) const;
    std::uint8_t readRegister(std::uint8_t reg);
    void writeRegister(std::uint8_t reg, std::uint8_t value);
    std::span<const std::uint8_t> transfer(std::uint8_t reg, int length);
    void transferRaw(std::size_t length);
    void throwIfDisposed() const;

    int m_fd = -1;
    Icm42688Settings m_settings;
    std::array<std::uint8_t, 1 + FifoSize> m_tx{};
    std::array<std::uint8_t, 1 + FifoSize> m_rx{};
    float m_gyroScale;
    float m_accelScale;
    bool m_disposed = false;
};

}  // namespace Sarwave::Core::Imu::Device
