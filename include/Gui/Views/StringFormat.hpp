#pragma once

#include <QtQml/qqmlregistration.h>

#include <QObject>
#include <QString>

namespace Sarwave::Core::Gui::Views {

/// The AXAML StringFormat bindings for QML: numbers come out exactly as .NET formats them.
class StringFormat : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

   public:
    using QObject::QObject;

    /// A double through a "0.00" / "0.##" / "0.#" style format: `minDecimals` mandatory
    /// decimals, up to `maxDecimals`.
    Q_INVOKABLE static QString fixed(double value, int minDecimals, int maxDecimals);
};

}  // namespace Sarwave::Core::Gui::Views
