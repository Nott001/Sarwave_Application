#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include <QObject>
#include <QString>
#include <memory>

#include "DevEnv/Gui/DialogSupport.hpp"
#include "MMWave/Porting/Context.hpp"

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Port-selection dialogue. Opens the chosen ports, validates the CLI port against an mmWave
/// device, prints the device version, then reveals a "Next" button.
/// Usage: `auto ctx = PortDialogue::showAsync(ownerWindow, engine);`
/// Returns null if the user cancelled. The caller owns (and must dispose) the context.
///
/// The object is the dialogue's logic; PortDialogue.qml is its look and binds to it.
class PortDialogue : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by PortDialogue::showAsync")

    Q_PROPERTY(QString defaultPortDirectory READ defaultPortDirectory CONSTANT)
    Q_PROPERTY(QString outputText READ outputText NOTIFY outputTextChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool nextVisible READ nextVisible NOTIFY nextVisibleChanged)

   public:
    static std::unique_ptr<Core::MMWave::Porting::Context> showAsync(QQuickWindow* owner,
                                                                     QQmlEngine& engine);

    ~PortDialogue() override;

    QString defaultPortDirectory() const;
    QString outputText() const {
        return m_outputText;
    }
    QString errorText() const {
        return m_errorText;
    }
    bool busy() const {
        return m_busy;
    }
    bool nextVisible() const {
        return m_nextVisible;
    }

    /// The Connect button. `separate` picks the two explicit names over common name + numbers.
    Q_INVOKABLE void connectPorts(bool separate, const QString& commonName, int cliNumber,
                                  int dataNumber, const QString& cliName, const QString& dataName);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void next();

    /// Called by the window when it closes, however it was closed.
    Q_INVOKABLE void windowClosed();

   signals:
    void outputTextChanged();
    void errorTextChanged();
    void busyChanged();
    void nextVisibleChanged();
    void closeRequested();

   private:
    PortDialogue() = default;

    void setBusy(bool busy);
    void setError(const QString& message);
    void disposeContext();

    std::unique_ptr<Core::MMWave::Porting::Context> m_context;
    bool m_busy = false;
    bool m_closed = false;
    bool m_nextVisible = false;
    QString m_outputText;
    QString m_errorText;
    Detail::BackgroundRunner m_background;  // last: joined before the rest is destroyed
};

}  // namespace Sarwave::DevEnv::Gui
