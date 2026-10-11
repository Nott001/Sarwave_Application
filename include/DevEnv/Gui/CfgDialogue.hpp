#pragma once

#include <QtQmlIntegration/qqmlintegration.h>

#include <QObject>
#include <QString>
#include <QUrl>
#include <optional>
#include <string>

#include "DevEnv/Gui/DialogSupport.hpp"
#include "MMWave/Porting/Context.hpp"

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::DevEnv::Gui {

/// Configuration-file selection dialogue (*.cfg only).
/// `CfgDialogue::showAsync(owner, engine)` validates the selection and closes (was cfgGUI).
/// `CfgDialogue::showAsync(owner, engine, ctx)` also streams the file to the device, echoing each
/// command/response into the output pane, then shows "Next" (was cfgGUIAndSend).
/// Returns the chosen path, or nullopt if the user cancelled. Errors keep the dialogue open.
///
/// The object is the dialogue's logic; CfgDialogue.qml is its look and binds to it.
class CfgDialogue : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Created by CfgDialogue::showAsync")

    Q_PROPERTY(QString initialPath READ initialPath CONSTANT)
    Q_PROPERTY(QString outputText READ outputText NOTIFY outputTextChanged)
    Q_PROPERTY(QString errorText READ errorText NOTIFY errorTextChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
    Q_PROPERTY(bool nextVisible READ nextVisible NOTIFY nextVisibleChanged)

   public:
    /// owner: parent window (the dialogue is modal to it).
    /// ctx: if given, the chosen config is sent to the device (without its sensorStart line).
    /// prefillPath: optional config path found ahead of time.
    static std::optional<std::string> showAsync(
        QQuickWindow* owner, QQmlEngine& engine, Core::MMWave::Porting::Context* ctx = nullptr,
        const std::optional<std::string>& prefillPath = std::nullopt);

    QString initialPath() const {
        return m_initialPath;
    }
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

    /// The Apply Config button, with the path field's text.
    Q_INVOKABLE void apply(const QString& pathText);
    Q_INVOKABLE void cancel();
    Q_INVOKABLE void next();
    Q_INVOKABLE void windowClosed();

    /// Where the file picker opens: the folder of the path typed so far, if it exists.
    Q_INVOKABLE QUrl browseStartFolder(const QString& pathText) const;

    /// A picked file as a local path.
    Q_INVOKABLE QString localPath(const QUrl& picked) const;

    /// A picker gives a real path; a hand-typed entry may be a file:// URL or plain path.
    Q_INVOKABLE static QString normalisePath(const QString& raw);

    /// Shows an error raised by the window itself (e.g. the file picker failing).
    Q_INVOKABLE void showError(const QString& message);

   signals:
    void outputTextChanged();
    void errorTextChanged();
    void busyChanged();
    void nextVisibleChanged();
    void closeRequested();

   private:
    CfgDialogue(Core::MMWave::Porting::Context* ctx, const std::optional<std::string>& prefillPath);

    /// Same layout as the terminal front-end: "-> cmd" then "   : response line".
    void appendCliReport(const std::string& line, const std::string& response);
    void setBusy(bool busy);
    void setError(const QString& message);

    Core::MMWave::Porting::Context* m_ctx;
    QString m_initialPath;
    QString m_log;
    std::optional<std::string> m_result;
    bool m_busy = false;
    bool m_closed = false;
    bool m_nextVisible = false;
    QString m_outputText;
    QString m_errorText;
    Detail::BackgroundRunner m_background;  // last: joined before the rest is destroyed
};

}  // namespace Sarwave::DevEnv::Gui
