#include "Gui/Views/StringFormat.hpp"

#include "MMWave/Common/DotNetFormat.hpp"

namespace Sarwave::Core::Gui::Views {

QString StringFormat::fixed(double value, int minDecimals, int maxDecimals) {
    return QString::fromStdString(MMWave::DotNet::formatFixed(value, minDecimals, maxDecimals));
}

}  // namespace Sarwave::Core::Gui::Views
