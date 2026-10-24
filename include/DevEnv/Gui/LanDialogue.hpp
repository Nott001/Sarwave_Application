#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include <QObject>
#include <QString>
#include <optional>
#include <string>

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Asks for the LAN address to stream on (sender) or to connect to (receiver), and the port.
/// Remembers each side's last answer.
/// Usage: `auto address = LanDialogue::showAsync(ownerWindow, engine, LanDialogue::Role::Receive);`
/// Returns nullopt if the user declined.
///
/// The object is the dialogue's logic; LanDialogueWindow.qml is its look and binds to it.
class LanDialogue : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by LanDialogue::showAsync")

    Q_PROPERTY(QString title READ title CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString addressLabel READ addressLabel CONSTANT)
    Q_PROPERTY(QString cancelText READ cancelText CONSTANT)
    Q_PROPERTY(QString okText READ okText CONSTANT)
    Q_PROPERTY(QString localAddresses READ localAddresses CONSTANT)
    Q_PROPERTY(QString initialHost READ initialHost CONSTANT)
    Q_PROPERTY(int initialPort READ initialPort CONSTANT)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)

   public:
    enum class Role {
        /// The live app, offering its session: asks which of its addresses to listen on.
        Send,
        /// The wireless app: asks for the sender's address.
        Receive,
    };

    struct Endpoint {
        std::string host;
        int port = 0;
    };

    static std::optional<Endpoint> showAsync(QQuickWindow* owner, QQmlEngine& engine, Role role);

    [[nodiscard]] QString title() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] QString addressLabel() const;
    [[nodiscard]] QString cancelText() const;
    [[nodiscard]] QString okText() const;
    [[nodiscard]] QString localAddresses() const;
    [[nodiscard]] QString initialHost() const {
        return m_initialHost;
    }
    [[nodiscard]] int initialPort() const {
        return m_initialPort;
    }
    [[nodiscard]] QString errorText() const {
        return m_errorText;
    }

    /// The OK button (Stream / Connect).
    Q_INVOKABLE void ok(const QString& address, int port);
    Q_INVOKABLE void cancel();

    /// This device's IPv4 addresses on networks that are up, loopback left out, each with its
    /// network's name (wlan0, eth0, ...) so the right one is easy to pick. One per line.
    static QString deviceAddresses();

   signals:
    void errorTextChanged();
    void closeRequested();

   private:
    explicit LanDialogue(Role role);

    [[nodiscard]] bool sending() const {
        return m_role == Role::Send;
    }
    void setError(const QString& message);

    Role m_role;
    QString m_initialHost;
    int m_initialPort = 0;
    QString m_errorText;
    std::optional<Endpoint> m_result;
};

}  // namespace Sarwave::DevEnv::Gui
