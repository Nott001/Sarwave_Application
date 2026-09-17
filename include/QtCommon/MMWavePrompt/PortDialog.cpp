#include "QtCommon/MMWavePrompt/PortDialog.hpp"

#include "MMWave/Prompting/General.hpp"

#include <QEventLoop>
#include <QGuiApplication>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QUrl>
#include <QVariant>

#include <boost/system/system_error.hpp>

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
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

    // Reads the port selections off the dialog window: either a "separate"
    // mode (explicit CLI/data names) or a shared common name + two numbers.
    General::MMWavePortPaths portPathsFrom(QQuickWindow &window) {
        if (window.property("separate").toBool()) {
            return {
                window.property("cliName").toString().toStdString(),
                window.property("dataName").toString().toStdString(),
            };
        }

        const std::string commonName =
            window.property("commonName").toString().toStdString();
        return {
            commonName + std::to_string(window.property("cliNumber").toInt()),
            commonName + std::to_string(window.property("dataNumber").toInt()),
        };
    }

} // namespace

    MMWave::Porter::Context portGUI() {
        if (!qGuiApp) {
            throw std::runtime_error("PortDialog requires a QGuiApplication instance");
        }

        ::qInitResources_MMWavePromptResources();

        QQmlEngine engine;
        QQmlComponent component(
            &engine,
            QUrl(QStringLiteral("qrc:/QtCommon/MMWavePrompt/PortDialog.qml")));

        if (component.isError()) {
            throw std::runtime_error(
                "Failed to load PortDialog.qml: " + component.errorString().toStdString());
        }

        auto *root = component.create();
        if (!root) {
            throw std::runtime_error(
                "Failed to create PortDialog: " + component.errorString().toStdString());
        }

        auto *window = qobject_cast<QQuickWindow *>(root);
        if (!window) {
            delete root;
            throw std::runtime_error("PortDialog root item is not a window");
        }

        QEventLoop loop;
        std::unique_ptr<MMWave::Porter::Context> result;

        OkHandler okHandler([&]() {
            try {
                const auto paths = portPathsFrom(*window);

                auto trial = std::make_unique<MMWave::Porter::Context>(paths.cli, paths.data);

                if (MMWave::Porter::validateCliPort(*trial)) {
                    std::string version = MMWave::Porter::getVersion(*trial);
                    std::erase(version, '\r');

                    const auto firstNonWs = version.find_first_not_of(" \t\n");
                    const auto lastNonWs = version.find_last_not_of(" \t\n");
                    if (firstNonWs == std::string::npos) {
                        version.clear();
                    } else {
                        version =
                            version.substr(firstNonWs, lastNonWs - firstNonWs + 1);
                    }

                    result = std::move(trial);
                    window->setProperty("didAccept", true);
                    window->setProperty("errorMessage", QString());
                    window->setProperty("outputText", QString::fromStdString(version));
                } else {
                    window->setProperty(
                        "errorMessage",
                        QStringLiteral(
                            "CLI port is not responding to an mmWave device. "
                            "Check the port selection and try again."));
                }
            }
            catch (const boost::wrapexcept<boost::system::system_error> &e) {
                window->setProperty(
                    "errorMessage",
                    QStringLiteral("Serial port does not exist: ") +
                        QString::fromStdString(e.code().message()));
            }
            catch (const std::exception &e) {
                window->setProperty("errorMessage", QString::fromStdString(e.what()));
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
            throw std::runtime_error("Port selection was cancelled");
        }

        return std::move(*result);
    }

} // namespace MMWave::Prompting::Gui

#include "PortDialog.moc"