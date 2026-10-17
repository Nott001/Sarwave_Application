#pragma once

#include <cstdint>
#include <string>

#include "MMWave/Common/Exceptions.hpp"

namespace Sarwave::Core::Imu::Device {

/// The IMU did not answer as an ICM-42688-P should: missing, miswired, or not holding its
/// configuration.
class ImuDeviceException : public IOException {
   public:
    using IOException::IOException;
};

/// GYRO_FS_SEL: the gyroscope's full-scale range. The value is the register field.
enum class GyroRange : std::uint8_t {
    Dps2000 = 0,
    Dps1000 = 1,
    Dps500 = 2,
    Dps250 = 3,
    Dps125 = 4
};

/// ACCEL_FS_SEL: the accelerometer's full-scale range. The value is the register field.
enum class AccelRange : std::uint8_t { G16 = 0, G8 = 1, G4 = 2, G2 = 3 };

/// GYRO_ODR / ACCEL_ODR, shared by both sensors so every FIFO packet holds a matching pair.
/// Only rates a polled FIFO keeps up with comfortably are offered. The value is the register field.
enum class OutputDataRate : std::uint8_t { Hz1000 = 6, Hz500 = 15, Hz200 = 7, Hz100 = 8, Hz50 = 9 };

/// GYRO_UI_FILT_BW / ACCEL_UI_FILT_BW: the low-pass filter ahead of the output data rate. The
/// bandwidth is max(400 Hz, ODR) divided by the number in the name, except OdrDiv2, which is
/// ODR / 2. The value is the register field.
enum class UiFilterBandwidth : std::uint8_t {
    OdrDiv2 = 0,
    Div4 = 1,
    Div5 = 2,
    Div8 = 3,
    Div10 = 4,
    Div16 = 5,
    Div20 = 6,
    Div40 = 7,
};

/// How the ICM-42688-P is wired and configured. The defaults suit a handheld radar: 200 Hz gives
/// ten or more samples per radar frame, ±500 °/s and ±4 g keep a steadily held device well
/// resolved (0.015 °/s and 0.12 mg per count) while hand sweeps still read clearly as motion,
/// and a 50 Hz filter passes hand tremor (under 15 Hz) while keeping the 200 Hz output free of
/// aliasing.
struct Icm42688Settings {
    /// SPI bus: 0 is /dev/spidev0.x, the header pins 19/21/23 on a Raspberry Pi.
    int busId = 0;

    /// Chip select: 0 is CE0 (pin 24), 1 is CE1 (pin 26).
    int chipSelectLine = 0;

    /// SPI clock. The part allows 24 MHz; jumper wires are happier lower.
    int clockFrequency = 8'000'000;

    GyroRange gyroRange = GyroRange::Dps500;
    AccelRange accelRange = AccelRange::G4;
    OutputDataRate rate = OutputDataRate::Hz200;
    UiFilterBandwidth filterBandwidth = UiFilterBandwidth::Div8;

    /// Counts per °/s at gyroRange.
    [[nodiscard]] float gyroSensitivity() const {
        return 32768.0f / static_cast<float>(2000 >> static_cast<int>(gyroRange));
    }

    /// Counts per g at accelRange.
    [[nodiscard]] float accelSensitivity() const {
        return 32768.0f / static_cast<float>(16 >> static_cast<int>(accelRange));
    }

    /// The nominal output data rate, in hertz.
    [[nodiscard]] float rateHz() const {
        switch (rate) {
            case OutputDataRate::Hz1000:
                return 1000;
            case OutputDataRate::Hz500:
                return 500;
            case OutputDataRate::Hz200:
                return 200;
            case OutputDataRate::Hz100:
                return 100;
            case OutputDataRate::Hz50:
                return 50;
        }
        throw ArgumentOutOfRangeException("Rate");
    }
};

}  // namespace Sarwave::Core::Imu::Device
