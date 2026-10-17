#pragma once

#include <cstdint>

namespace Sarwave::Core::Imu::Device {

/// The ICM-42688-P registers this driver touches, all in user bank 0 (TDK InvenSense
/// DS-000347). Over SPI the top bit of the address byte selects a read (1) or a write (0).
namespace Icm42688Registers {

inline constexpr std::uint8_t ReadFlag = 0x80;

inline constexpr std::uint8_t DeviceConfig = 0x11;
inline constexpr std::uint8_t IntStatus = 0x2D;
inline constexpr std::uint8_t FifoCountH = 0x2E;
inline constexpr std::uint8_t FifoData = 0x30;
inline constexpr std::uint8_t SignalPathReset = 0x4B;
inline constexpr std::uint8_t IntfConfig0 = 0x4C;
inline constexpr std::uint8_t PwrMgmt0 = 0x4E;
inline constexpr std::uint8_t GyroConfig0 = 0x4F;
inline constexpr std::uint8_t AccelConfig0 = 0x50;
inline constexpr std::uint8_t GyroAccelConfig0 = 0x52;
inline constexpr std::uint8_t FifoConfig = 0x16;
inline constexpr std::uint8_t FifoConfig1 = 0x5F;
inline constexpr std::uint8_t WhoAmI = 0x75;
inline constexpr std::uint8_t RegBankSel = 0x76;

/// DEVICE_CONFIG: soft reset. The datasheet asks for 1 ms before the next access.
inline constexpr std::uint8_t SoftReset = 0x01;

/// INT_STATUS: the FIFO filled up. Clears when INT_STATUS is read.
inline constexpr std::uint8_t IntStatusFifoFull = 0x02;

/// SIGNAL_PATH_RESET: empties the FIFO.
inline constexpr std::uint8_t FifoFlush = 0x02;

/// INTF_CONFIG0: FIFO count and sensor data big endian (the reset values), and the I2C interface
/// disabled, which TDK recommends when the part is only ever driven over SPI.
inline constexpr std::uint8_t IntfConfig0SpiOnly = 0x33;

/// FIFO_CONFIG: stream-to-FIFO mode.
inline constexpr std::uint8_t FifoModeStream = 0x40;

/// FIFO_CONFIG1: accelerometer, gyroscope, temperature and timestamp in every packet, which
/// makes each one the 16-byte "packet 3" of the datasheet.
inline constexpr std::uint8_t FifoConfig1Packet3 = 0x0F;

/// PWR_MGMT0: gyroscope and accelerometer in low-noise mode, temperature sensor on.
inline constexpr std::uint8_t PwrMgmt0LowNoise = 0x0F;

inline constexpr std::uint8_t PwrMgmt0Off = 0x00;

/// FIFO packet header bit set when the FIFO had nothing to give.
inline constexpr std::uint8_t FifoHeaderEmpty = 0x80;

/// FIFO packet header bits set when the packet carries accelerometer and gyroscope data.
inline constexpr std::uint8_t FifoHeaderAccelGyro = 0x60;

/// What a sensor axis reads while its data is not valid yet (start-up, or no sample).
inline constexpr std::int16_t InvalidSample = -32768;

}  // namespace Icm42688Registers
}  // namespace Sarwave::Core::Imu::Device
