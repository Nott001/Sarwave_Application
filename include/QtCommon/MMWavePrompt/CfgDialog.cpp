#include "QtCommon/MMWavePrompt/CfgDialog.hpp"

#include <QEventLoop>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QUrl>
#include <QVariant>

#include <filesystem>
#include <functional>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <utility>

extern int qInitResources_MMWavePromptResources();

namespace MMWave::Prompting::Gui {
namespace {

    // The dialog declares okRequested() as a pure QML signal, which has no C++
    // member we can connect to with the pointer-to-member syntax. This small
    // dispatcher exposes a real slot so the string-based SIGNAL()/SLOT()
    // connection (the only form that accepts QML-only signals) can run the
    // same validation logic.
    class OkHandler final : public QObject {
        Q_OBJECT
    public:
        explicit OkHandler(std::function<void()> onOk, QObject *parent = nullptr)
            : QObject(parent), m_onOk(std::move(onOk)) {}

    public slots:
        void invoke() {
            if (m_onOk) {
                m_onOk();
            }
        }

    private:
        std::function<void()> m_onOk;
    };

    // The FileDialog hands back a file:// URL; a manually typed entry is a
    // plain path. Normalise both to a local filesystem path.
    std::string cfgPathFrom(const QQuickWindow &window) {
        const QString raw = window.property("cfgPath").toString();
        const QUrl url(raw);
        return (url.isLocalFile() ? url.toLocalFile() : raw).toStdString();
    }

    // Appends one command/response pair to the output pane, using the same
    // layout as the terminal front-end.
    void appendCliReport(std::string &output,
                         const std::string &line,
                         const std::string &response) {
        output += "-> " + line + "\n";

        std::string cleaned = response;
        std::erase(cleaned, '\r');

        std::istringstream responseStream(cleaned);
        std::string responseLine;
        bool first = true;
        while (std::getline(responseStream, responseLine)) {
            if (responseLine.empty() && first) continue;
            output += "   : " + responseLine + "\n";
            first = false;
        }
    }

    // sendConfigFile reports progress through a plain function pointer, so the
    // target dialog/output are stashed here for the duration of the (blocking,
    // single-threaded) send.
    struct ReportSink {
        QQuickWindow *window = nullptr;
        std::string *output = nullptr;
    };
    ReportSink g_reportSink;

    void streamCliReport(const std::string &line, const std::string &response) {
        if (!g_reportSink.window || !g_reportSink.output) return;
        appendCliReport(*g_reportSink.output, line, response);
        g_reportSink.window->setProperty(
            "outputText", QString::fromStdString(*g_reportSink.output));
        QGuiApplication::processEvents();
    }

    // Loads the dialog and runs its event loop. When ctx is non-null the
    // selected config is sent through MMWave::Porter::sendConfigFile with the
    // report streamed into the dialog; otherwise the selection is merely
    // validated and the dialog closes as before. Returns the chosen path.
    std::string runCfgDialog(MMWave::Porter::Context *ctx,
                             const std::string &prefillPath) {
        if (!qGuiApp) {
            throw std::runtime_error(
                "CfgDialog requires a QGuiApplication instance");
        }

        ::qInitResources_MMWavePromptResources();

        QQmlEngine engine;
        QQmlComponent component(
            &engine,
            QUrl(QStringLiteral("qrc:/QtCommon/MMWavePrompt/CfgDialog.qml")));

        if (component.isError()) {
            throw std::runtime_error(
                "Failed to load CfgDialog.qml: " + component.errorString().toStdString());
        }

        auto *root = component.create();
        if (!root) {
            throw std::runtime_error(
                "Failed to create CfgDialog: " + component.errorString().toStdString());
        }

        auto *window = qobject_cast<QQuickWindow *>(root);
        if (!window) {
            delete root;
            throw std::runtime_error("CfgDialog root item is not a window");
        }

        if (!prefillPath.empty()) {
            window->setProperty("cfgPath", QString::fromStdString(prefillPath));
        }

        QEventLoop loop;
        std::unique_ptr<std::string> result;

        OkHandler okHandler([&]() {
            try {
                std::string path = cfgPathFrom(*window);

                if (path.empty()) {
                    window->setProperty(
                        "errorMessage",
                        QStringLiteral("Please select a configuration file."));
                    return;
                }

                std::error_code ec;
                if (!std::filesystem::is_regular_file(path, ec) || ec) {
                    window->setProperty(
                        "errorMessage",
                        QStringLiteral("Configuration file does not exist: ") +
                            QString::fromStdString(path));
                    return;
                }

                window->setProperty("didAccept", false);
                window->setProperty("errorMessage", QString());
                window->setProperty("outputText", QString());

                if (ctx) {
                    std::string output;
                    g_reportSink.window = window;
                    g_reportSink.output = &output;
                    MMWave::Porter::sendConfigFile(*ctx, path, &streamCliReport);
                    g_reportSink.window = nullptr;
                    g_reportSink.output = nullptr;
                }

                result = std::make_unique<std::string>(std::move(path));
                window->setProperty("didAccept", true);
                window->setProperty("errorMessage", QString());

                if (!ctx) {
                    loop.quit();
                    window->close();
                }
            }
            catch (const std::exception &e) {
                window->setProperty("didAccept", false);
                window->setProperty(
                    "errorMessage", QString::fromStdString(e.what()));
            }
        });

        QObject::connect(window, SIGNAL(okRequested()), &okHandler, SLOT(invoke()));
        QObject::connect(window, SIGNAL(acceptedByUser()), &loop, SLOT(quit()));
        QObject::connect(window, SIGNAL(rejectedByUser()), &loop, SLOT(quit()));

        window->show();
        loop.exec();
        window->close();

        if (auto *focus = window->activeFocusItem()) {
            focus->setFocus(false);
        }
        QGuiApplication::processEvents();

        delete root;

        if (!result) {
            throw std::runtime_error("Configuration selection was cancelled");
        }

        return std::move(*result);
    }

} // namespace

    std::string cfgGUI() {
        return runCfgDialog(nullptr, {});
    }

    std::string cfgGUIAndSend(MMWave::Porter::Context &ctx,
                              const std::string &path) {
        return runCfgDialog(&ctx, path);
    }

} // namespace MMWave::Prompting::Gui

#include "CfgDialog.moc"