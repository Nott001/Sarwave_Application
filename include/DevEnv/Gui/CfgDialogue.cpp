#include "DevEnv/Gui/CfgDialogue.hpp"

#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <filesystem>
#include <memory>

namespace Sarwave::DevEnv::Gui {

using Core::MMWave::Porting::Context;

std::optional<std::string> CfgDialogue::showAsync(QQuickWindow* owner, QQmlEngine& engine,
                                                  Context* ctx,
                                                  const std::optional<std::string>& prefillPath) {
    std::unique_ptr<CfgDialogue> dialog(new CfgDialogue(ctx, prefillPath));
    Detail::showDialog(engine, owner, "CfgDialogueWindow", dialog.get());
    dialog->m_closed = true;
    return dialog->m_result;
}

CfgDialogue::CfgDialogue(Context* ctx, const std::optional<std::string>& prefillPath)
    : m_ctx(ctx), m_initialPath(prefillPath ? QString::fromStdString(*prefillPath) : QString()) {
}

void CfgDialogue::apply(const QString& pathText) {
    if (m_busy) return;

    const QString path = normalisePath(pathText);
    if (path.isEmpty()) {
        setError(QStringLiteral("Please select a configuration file."));
        return;
    }
    if (!std::filesystem::is_regular_file(path.toStdString())) {
        setError(QStringLiteral("Configuration file does not exist: ") + path);
        return;
    }

    setBusy(true);
    setError({});
    m_result.reset();
    if (m_nextVisible) {
        m_nextVisible = false;
        emit nextVisibleChanged();
    }
    m_log.clear();
    m_outputText.clear();
    emit outputTextChanged();

    const std::string chosen = path.toStdString();
    const auto finish = [this, chosen](Detail::Outcome<bool> outcome) {
        try {
            outcome.get();
            m_result = chosen;
            if (!m_ctx) {
                emit closeRequested();
            } else {
                m_nextVisible = true;
                emit nextVisibleChanged();
            }
        } catch (const std::exception& e) {
            m_result.reset();
            setError(QString::fromStdString(e.what()));
        }
        setBusy(false);
    };

    if (!m_ctx) {
        finish(Detail::Outcome<bool>::fromValue(true));
        return;
    }

    // Blocking serial I/O on a worker thread; each report is marshalled back so the output pane
    // updates live and the window stays responsive.
    QPointer<CfgDialogue> self(this);
    Context* ctx = m_ctx;
    m_background.run(
        this,
        [ctx, chosen, self]() {
            ctx->sendConfigFile(
                chosen,
                [self](const std::string& line, const std::string& response) {
                    QMetaObject::invokeMethod(
                        self.data(),
                        [self, line, response] {
                            if (self) self->appendCliReport(line, response);
                        },
                        Qt::QueuedConnection);
                },
                true);
            return true;
        },
        finish);
}

void CfgDialogue::cancel() {
    m_result.reset();
    emit closeRequested();
}

void CfgDialogue::next() {
    emit closeRequested();
}

void CfgDialogue::windowClosed() {
    m_closed = true;
}

QUrl CfgDialogue::browseStartFolder(const QString& pathText) const {
    const QString current = normalisePath(pathText);
    const QString folder = current.isEmpty() ? QString() : QFileInfo(current).path();
    if (!folder.isEmpty() && QDir(folder).exists()) return QUrl::fromLocalFile(folder);
    return {};
}

QString CfgDialogue::localPath(const QUrl& picked) const {
    return picked.isLocalFile() ? picked.toLocalFile() : picked.toString();
}

QString CfgDialogue::normalisePath(const QString& raw) {
    const QString trimmed = raw.trimmed();
    const QUrl url(trimmed, QUrl::StrictMode);
    if (url.isValid() && url.isLocalFile() &&
        trimmed.startsWith(QStringLiteral("file:"), Qt::CaseInsensitive)) {
        return url.toLocalFile();
    }
    return trimmed;
}

void CfgDialogue::showError(const QString& message) {
    setError(message);
}

void CfgDialogue::appendCliReport(const std::string& line, const std::string& response) {
    m_log += QStringLiteral("-> ") + Detail::fromDeviceText(line) + QLatin1Char('\n');

    QString cleaned = Detail::fromDeviceText(response);
    cleaned.remove(QLatin1Char('\r'));

    bool first = true;
    for (const QString& respLine : cleaned.split(QLatin1Char('\n'))) {
        if (respLine.isEmpty() && first) continue;
        m_log += QStringLiteral("   : ") + respLine + QLatin1Char('\n');
        first = false;
    }

    m_outputText = m_log;
    emit outputTextChanged();
}

void CfgDialogue::setBusy(bool busy) {
    if (m_busy == busy) return;
    m_busy = busy;
    emit busyChanged();
}

void CfgDialogue::setError(const QString& message) {
    if (m_errorText == message) return;
    m_errorText = message;
    emit errorTextChanged();
}

}  // namespace Sarwave::DevEnv::Gui
