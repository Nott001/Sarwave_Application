#include "DevEnv/Gui/LanDialogue.hpp"

#include <QNetworkInterface>
#include <QStringList>
#include <boost/asio/ip/address.hpp>

#include "DevEnv/ConfigMemory.hpp"
#include "DevEnv/Gui/DialogSupport.hpp"
#include "Integration/PeopleTracking/Lan/LanSender.hpp"

namespace Sarwave::DevEnv::Gui {

using Integration::PeopleTracking::Lan::LanSender;

std::optional<LanDialogue::Endpoint> LanDialogue::showAsync(QQuickWindow* owner, QQmlEngine& engine,
                                                            Role role) {
    LanDialogue dialog(role);
    Detail::showDialog(engine, owner, "LanDialogueWindow", &dialog);
    return dialog.m_result;
}

LanDialogue::LanDialogue(Role role) : m_role(role) {
    const auto remembered = ConfigMemory::getLanAddress(sending());
    m_initialHost = remembered  ? QString::fromStdString(remembered->host)
                    : sending() ? QStringLiteral("0.0.0.0")
                                : QString();
    m_initialPort = remembered ? remembered->port : LanSender::DefaultPort;
}

QString LanDialogue::title() const {
    return sending() ? QStringLiteral("Stream to a Wireless Dashboard")
                     : QStringLiteral("Connect to the Sensor");
}

QString LanDialogue::description() const {
    return sending()
               ? QStringLiteral(
                     "Offer this session on the network, for the wireless app to show: enter this "
                     "device's address there. 0.0.0.0 listens on every network this device is on; "
                     "one of its addresses, on that one alone.")
               : QStringLiteral(
                     "Enter the IP address (or host name) of the device running the live app, and "
                     "the port it streams on. Its address is shown in its own prompt.");
}

QString LanDialogue::addressLabel() const {
    return sending() ? QStringLiteral("Listen on:") : QStringLiteral("Sender address:");
}

QString LanDialogue::cancelText() const {
    return sending() ? QStringLiteral("Don't Stream") : QStringLiteral("Cancel");
}

QString LanDialogue::okText() const {
    return sending() ? QStringLiteral("Stream") : QStringLiteral("Connect");
}

QString LanDialogue::localAddresses() const {
    const QString addresses = deviceAddresses();
    return addresses.isEmpty() ? QStringLiteral("None: this device is not on a network.")
                               : addresses;
}

QString LanDialogue::deviceAddresses() {
    QStringList lines;
    for (const QNetworkInterface& network : QNetworkInterface::allInterfaces()) {
        const auto flags = network.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp) ||
            flags.testFlag(QNetworkInterface::IsLoopBack))
            continue;
        for (const QNetworkAddressEntry& entry : network.addressEntries()) {
            if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
            lines << entry.ip().toString().leftJustified(15) + QStringLiteral("  (") +
                         network.name() + QStringLiteral(")");
        }
    }
    return lines.join(QLatin1Char('\n'));
}

void LanDialogue::ok(const QString& address, int port) {
    const std::string host = address.trimmed().toStdString();

    boost::system::error_code notAnAddress;
    if (sending() && (boost::asio::ip::make_address(host, notAnAddress), notAnAddress)) {
        setError(
            QStringLiteral("Enter one of this device's IP addresses, or 0.0.0.0 for all of them."));
        return;
    }

    if (host.empty()) {
        setError(QStringLiteral("Enter the sender's IP address or host name."));
        return;
    }

    if (port < 1 || port > 65535) {
        setError(QStringLiteral("Enter a port from 1 to 65535."));
        return;
    }

    ConfigMemory::saveLanAddress(sending(), host, port);
    m_result = Endpoint{host, port};
    emit closeRequested();
}

void LanDialogue::cancel() {
    emit closeRequested();
}

void LanDialogue::setError(const QString& message) {
    if (m_errorText == message) return;
    m_errorText = message;
    emit errorTextChanged();
}

}  // namespace Sarwave::DevEnv::Gui
