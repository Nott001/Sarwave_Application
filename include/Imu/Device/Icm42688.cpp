#include "Imu/Device/Icm42688.hpp"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <thread>
#include <utility>

#include "Imu/Device/Icm42688Registers.hpp"

#if defined(__linux__)
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace Sarwave::Core::Imu::Device {
namespace {

namespace R = Icm42688Registers;

std::int16_t readInt16BigEndian(std::span<const std::uint8_t> b) {
    return static_cast<std::int16_t>(static_cast<std::uint16_t>(b[0] << 8 | b[1]));
}

std::string hex2(unsigned value) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << value;
    return out.str();
}

std::string spiPath(const Icm42688Settings& settings) {
    return "/dev/spidev" + std::to_string(settings.busId) + "." +
           std::to_string(settings.chipSelectLine);
}

}  // namespace

std::unique_ptr<Icm42688> Icm42688::open(const Icm42688Settings& settings) {
#if defined(__linux__)
    const std::string path = spiPath(settings);
    const int fd = ::open(path.c_str(), O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        const int error = errno;
        std::string hint;
        if (error == ENOENT) hint = " Enable SPI (dtparam=spi=on in /boot/firmware/config.txt).";
        if (error == EACCES) hint = " Join the \"spi\" group.";
        throw IOException("Could not open the SPI device " + path + ": " + std::strerror(error) +
                          "." + hint);
    }

    // CPOL = 1, CPHA = 1. Mode 0 also works on this part, but Mode 3 is what TDK specifies.
    std::uint8_t mode = SPI_MODE_3;
    std::uint8_t bits = 8;
    auto speed = static_cast<std::uint32_t>(settings.clockFrequency);
    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0 || ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
        const int error = errno;
        ::close(fd);
        throw IOException("Could not configure the SPI device " + path + ": " +
                          std::strerror(error) + ".");
    }

    std::unique_ptr<Icm42688> device(new Icm42688(fd, settings));
    // On failure the device's destructor powers it down (if it can) and closes the file.
    device->initialize();
    return device;
#else
    (void)settings;
    throw IOException("Linux SPI (spidev) is not available on this platform.");
#endif
}

Icm42688::Icm42688(int fd, const Icm42688Settings& settings)
    : m_fd(fd),
      m_settings(settings),
      m_gyroScale(1.0f / settings.gyroSensitivity()),
      m_accelScale(1.0f / settings.accelSensitivity()) {
}

Icm42688::~Icm42688() {
    dispose();
}

void Icm42688::throwIfDisposed() const {
    if (m_disposed) throw ObjectDisposedException("Sarwave.Core.Imu.Device.Icm42688");
}

std::uint8_t Icm42688::readWhoAmI() {
    return readRegister(R::WhoAmI);
}

int Icm42688::readFifo(std::span<Icm42688Reading> destination, bool& overflowed) {
    throwIfDisposed();

    // INT_STATUS, FIFO_COUNTH and FIFO_COUNTL are adjacent: one transfer reads all three, and
    // reading COUNTH first is what latches COUNTL to go with it.
    const auto status = transfer(R::IntStatus, 3);
    overflowed = (status[0] & R::IntStatusFifoFull) != 0;
    if (overflowed) {
        writeRegister(R::SignalPathReset, R::FifoFlush);
        return 0;
    }

    const int available = std::min(static_cast<int>(status[1] << 8 | status[2]), FifoSize);
    const int packets = std::min(available / FifoPacketSize, static_cast<int>(destination.size()));
    if (packets == 0) return 0;

    // FIFO_DATA does not auto-increment: a burst read keeps draining the FIFO. Only whole
    // packets are read, since a packet read in part is dropped by the chip.
    const auto data = transfer(R::FifoData, packets * FifoPacketSize);

    int count = 0;
    for (std::size_t offset = 0; offset < data.size(); offset += FifoPacketSize) {
        Icm42688Reading reading;
        if (tryDecode(data.subspan(offset, FifoPacketSize), reading))
            destination[static_cast<std::size_t>(count++)] = reading;
    }
    return count;
}

void Icm42688::flushFifo() {
    throwIfDisposed();
    writeRegister(R::SignalPathReset, R::FifoFlush);
}

void Icm42688::dispose() {
    if (m_disposed) return;

    try {
        writeRegister(R::PwrMgmt0, R::PwrMgmt0Off);
    } catch (const IOException&) {
        // Unplugged or the bus already gone: nothing left to power down.
    }

    m_disposed = true;
#if defined(__linux__)
    if (m_fd >= 0) ::close(m_fd);
#endif
    m_fd = -1;
}

void Icm42688::initialize() {
    writeRegister(R::RegBankSel, 0);
    requireWhoAmI();

    writeRegister(R::DeviceConfig, R::SoftReset);
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    requireWhoAmI();
    readRegister(R::IntStatus);  // clears RESET_DONE

    // Configured with the sensors still off, which is when the datasheet allows it.
    const Icm42688Settings& s = m_settings;
    const auto filter =
        static_cast<std::uint8_t>(static_cast<std::uint8_t>(s.filterBandwidth) << 4 |
                                  static_cast<std::uint8_t>(s.filterBandwidth));
    const std::pair<std::uint8_t, std::uint8_t> configuration[] = {
        {R::IntfConfig0, R::IntfConfig0SpiOnly},
        {R::GyroConfig0, static_cast<std::uint8_t>(static_cast<std::uint8_t>(s.gyroRange) << 5 |
                                                   static_cast<std::uint8_t>(s.rate))},
        {R::AccelConfig0, static_cast<std::uint8_t>(static_cast<std::uint8_t>(s.accelRange) << 5 |
                                                    static_cast<std::uint8_t>(s.rate))},
        {R::GyroAccelConfig0, filter},
        {R::FifoConfig1, R::FifoConfig1Packet3},
        {R::FifoConfig, R::FifoModeStream},
    };

    for (const auto& [reg, value] : configuration) writeRegister(reg, value);

    // Read back: a broken MOSI line still lets WHO_AM_I through but loses every write.
    for (const auto& [reg, value] : configuration) {
        const std::uint8_t actual = readRegister(reg);
        if (actual != value) {
            throw ImuDeviceException("ICM-42688-P register 0x" + hex2(reg) + " reads back 0x" +
                                     hex2(actual) + " after writing 0x" + hex2(value) +
                                     "; check the MOSI wire.");
        }
    }

    writeRegister(R::PwrMgmt0, R::PwrMgmt0LowNoise);

    // No register access for 200 µs after PWR_MGMT0, and the gyroscope needs up to 45 ms before
    // its output is valid. What reached the FIFO in that time is thrown away.
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    writeRegister(R::SignalPathReset, R::FifoFlush);
}

void Icm42688::requireWhoAmI() {
    const std::uint8_t whoAmI = readWhoAmI();
    if (whoAmI == ExpectedWhoAmI) return;

    const std::string hint =
        whoAmI == 0x00 || whoAmI == 0xFF
            ? "Nothing is driving MISO: check power, ground, CS and the SPI wires, and that SPI is "
              "enabled (dtparam=spi=on)."
            : "Another device answered on this chip select.";

    throw ImuDeviceException("No ICM-42688-P on SPI " + std::to_string(m_settings.busId) + "." +
                             std::to_string(m_settings.chipSelectLine) + ": WHO_AM_I read 0x" +
                             hex2(whoAmI) + ", expected 0x" + hex2(ExpectedWhoAmI) + ". " + hint);
}

bool Icm42688::tryDecode(std::span<const std::uint8_t> packet, Icm42688Reading& reading) const {
    const std::uint8_t header = packet[0];
    const std::int16_t ax = readInt16BigEndian(packet.subspan(1));
    const std::int16_t gx = readInt16BigEndian(packet.subspan(7));

    if ((header & R::FifoHeaderEmpty) != 0 ||
        (header & R::FifoHeaderAccelGyro) != R::FifoHeaderAccelGyro || ax == R::InvalidSample ||
        gx == R::InvalidSample) {
        return false;
    }

    reading.acceleration =
        Vector3{static_cast<float>(ax), static_cast<float>(readInt16BigEndian(packet.subspan(3))),
                static_cast<float>(readInt16BigEndian(packet.subspan(5)))} *
        m_accelScale;
    reading.angularRate =
        Vector3{static_cast<float>(gx), static_cast<float>(readInt16BigEndian(packet.subspan(9))),
                static_cast<float>(readInt16BigEndian(packet.subspan(11)))} *
        m_gyroScale;

    // FIFO temperature is one signed byte: °C = value / 2.07 + 25.
    reading.temperature = static_cast<float>(static_cast<std::int8_t>(packet[13])) / 2.07f + 25.0f;
    reading.timestamp = static_cast<std::uint16_t>(packet[14] << 8 | packet[15]);
    return true;
}

std::uint8_t Icm42688::readRegister(std::uint8_t reg) {
    return transfer(reg, 1)[0];
}

void Icm42688::writeRegister(std::uint8_t reg, std::uint8_t value) {
    m_tx[0] = reg;
    m_tx[1] = value;
    transferRaw(2);
}

/// Reads `length` bytes starting at `reg`. Returns a view into the receive buffer, valid until
/// the next transfer.
std::span<const std::uint8_t> Icm42688::transfer(std::uint8_t reg, int length) {
    const auto n = static_cast<std::size_t>(length) + 1;
    std::fill_n(m_tx.begin(), n, std::uint8_t{0});
    m_tx[0] = static_cast<std::uint8_t>(reg | R::ReadFlag);
    transferRaw(n);
    return std::span<const std::uint8_t>(m_rx).subspan(1, static_cast<std::size_t>(length));
}

void Icm42688::transferRaw(std::size_t length) {
#if defined(__linux__)
    if (m_fd < 0) throw IOException("The SPI device is closed.");

    spi_ioc_transfer message{};
    message.tx_buf = reinterpret_cast<std::uintptr_t>(m_tx.data());
    message.rx_buf = reinterpret_cast<std::uintptr_t>(m_rx.data());
    message.len = static_cast<std::uint32_t>(length);
    message.speed_hz = static_cast<std::uint32_t>(m_settings.clockFrequency);
    message.bits_per_word = 8;
    if (ioctl(m_fd, SPI_IOC_MESSAGE(1), &message) < 0) {
        throw IOException(std::string("SPI transfer failed: ") + std::strerror(errno) + ".");
    }
#else
    (void)length;
    throw IOException("Linux SPI (spidev) is not available on this platform.");
#endif
}

}  // namespace Sarwave::Core::Imu::Device
