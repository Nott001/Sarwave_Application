#include "DevEnv/Gui/PortDialogue.hpp"

#include <utility>

#include "MMWave/Prompting/Prompting.hpp"

namespace Sarwave::DevEnv::Gui {

using Core::MMWave::Porting::Context;
using Core::MMWave::Porting::MmwavePortPaths;

std::unique_ptr<Context> PortDialogue::showAsync(QQuickWindow* owner, QQmlEngine& engine) {
    std::unique_ptr<PortDialogue> dialog(new PortDialogue());
    Detail::showDialog(engine, owner, "PortDialogueWindow", dialog.get());
    dialog->m_closed = true;
    return std::move(dialog->m_context);
}

PortDialogue::~PortDialogue() = default;

QString PortDialogue::defaultPortDirectory() const {
    return QString::fromStdString(Core::MMWave::Prompting::General::defaultPortDirectory());
}

void PortDialogue::connectPorts(bool separate, const QString& commonName, int cliNumber,
                                int dataNumber, const QString& cliName, const QString& dataName) {
    if (m_busy) return;

    setBusy(true);
    setError({});

    MmwavePortPaths paths;
    if (separate) {
        paths = {cliName.toStdString(), dataName.toStdString()};
    } else {
        const std::string common = commonName.toStdString();
        paths = {common + std::to_string(cliNumber), common + std::to_string(dataNumber)};
    }

    struct Trial {
        std::unique_ptr<Context> context;
        std::string version;
    };

    // Serial I/O blocks for up to a couple of seconds; keep the GUI thread free.
    m_background.run(
        this,
        [paths]() -> Trial {
            auto ctx = Context::openMmwavePorts(paths);
            std::string reply = ctx->getVersion();
            if (reply.find(Context::VersionMarker) == std::string::npos) return {};

            std::erase(reply, '\r');
            const auto first = reply.find_first_not_of(" \t\n\v\f");
            const auto last = reply.find_last_not_of(" \t\n\v\f");
            reply =
                first == std::string::npos ? std::string() : reply.substr(first, last - first + 1);
            return {std::move(ctx), std::move(reply)};
        },
        [this](Detail::Outcome<Trial> outcome) {
            try {
                Trial trial = outcome.get();
                if (m_closed) return;  // the dialogue is gone; the trial context closes here

                if (!trial.context) {
                    setError(
                        QStringLiteral("CLI port is not responding to an mmWave device. "
                                       "Check the port selection and try again."));
                } else {
                    disposeContext();  // replace any earlier successful connection
                    m_context = std::move(trial.context);
                    m_outputText = Detail::fromDeviceText(trial.version);
                    emit outputTextChanged();
                    m_nextVisible = true;
                    emit nextVisibleChanged();
                }
            } catch (const std::exception& e) {
                setError(QStringLiteral("Serial port unavailable: ") +
                         QString::fromStdString(e.what()));
            }
            setBusy(false);
        });
}

void PortDialogue::cancel() {
    disposeContext();
    emit closeRequested();
}

void PortDialogue::next() {
    emit closeRequested();
}

void PortDialogue::windowClosed() {
    m_closed = true;
}

void PortDialogue::setBusy(bool busy) {
    if (m_busy == busy) return;
    m_busy = busy;
    emit busyChanged();
}

void PortDialogue::setError(const QString& message) {
    if (m_errorText == message) return;
    m_errorText = message;
    emit errorTextChanged();
}

void PortDialogue::disposeContext() {
    m_context.reset();
}

}  // namespace Sarwave::DevEnv::Gui
