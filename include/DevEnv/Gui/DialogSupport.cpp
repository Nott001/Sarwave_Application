#include "DevEnv/Gui/DialogSupport.hpp"

#include <QEventLoop>
#include <QQmlComponent>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QScreen>
#include <memory>
#include <stdexcept>

namespace Sarwave::DevEnv::Gui::Detail {

void showDialog(QQmlEngine& engine, QQuickWindow* owner, const char* typeName,
                QObject* controller) {
    QQmlComponent component(&engine);
    component.loadFromModule("Sarwave.DevEnv", typeName);
    if (component.isError()) {
        throw std::runtime_error("Failed to load " + std::string(typeName) + ": " +
                                 component.errorString().toStdString());
    }

    std::unique_ptr<QObject> root(
        component.createWithInitialProperties({{"controller", QVariant::fromValue(controller)}}));
    auto* window = qobject_cast<QQuickWindow*>(root.get());
    if (!window) throw std::runtime_error(std::string(typeName) + " is not a window");

    window->setModality(Qt::ApplicationModal);
    if (owner) {
        window->setTransientParent(owner);
        const QRect frame = owner->geometry();
        window->setPosition(frame.x() + (frame.width() - window->width()) / 2,
                            frame.y() + (frame.height() - window->height()) / 2);
    }

    QEventLoop loop;
    QObject::connect(window, &QWindow::visibleChanged, &loop, [&loop](bool visible) {
        if (!visible) loop.quit();
    });
    window->show();
    loop.exec();
}

}  // namespace Sarwave::DevEnv::Gui::Detail
