#pragma once

#include <QQuickItem>

class QQmlEngine;
class QQuickWindow;

namespace Sarwave::Core::Gui::Models {
class RadarData;
}

namespace Sarwave::Core::Gui::Views {

/// Call once after the QGuiApplication exists: registers the embedded Inter + Cascadia Mono fonts
/// and makes Inter 14px the application font (Avalonia's defaults), and registers the
/// Sarwave.Gui QML types.
void initialize();

/// `window.Content = new MaximalGui(data)`: creates the dashboard item and its view model (owned
/// by the item; it unsubscribes from `data` when destroyed), and makes it fill the window's
/// content area, replacing content previously placed there.
QQuickItem* showMaximalGui(QQuickWindow& window, QQmlEngine& engine, Models::RadarData& data);

/// `window.Content = new MinimalGui(data)`.
QQuickItem* showMinimalGui(QQuickWindow& window, QQmlEngine& engine, Models::RadarData& data);

/// The dashboards with the previewer's sample data (Avalonia's design-time DataContext).
QQuickItem* showMaximalGuiDesign(QQuickWindow& window, QQmlEngine& engine);
QQuickItem* showMinimalGuiDesign(QQuickWindow& window, QQmlEngine& engine);

}  // namespace Sarwave::Core::Gui::Views
