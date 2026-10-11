#include "DevEnv/TextBoxLayoutExtensions.hpp"

#include <QFontMetricsF>

namespace Sarwave::DevEnv {

int getCharacterOverflow(const QString& text, const QFont& font, double availableWidth) {
    if (text.isEmpty()) return 0;

    if (availableWidth <= 0) return static_cast<int>(text.size());  // No space available at all

    const QFontMetricsF metrics(font);
    const auto width = [&](const QString& substring) {
        return metrics.horizontalAdvance(substring);
    };

    // Check if full string fits first
    if (width(text) <= availableWidth) return 0;

    // Binary search to quickly find the exact fitting character length
    int low = 0;
    int high = static_cast<int>(text.size());
    int maxFittingChars = 0;

    while (low <= high) {
        const int mid = (low + high) / 2;
        if (width(text.left(mid)) <= availableWidth) {
            maxFittingChars = mid;
            low = mid + 1;  // Try fitting more characters
        } else {
            high = mid - 1;  // Too long, reduce length
        }
    }

    return static_cast<int>(text.size()) - maxFittingChars;
}

}  // namespace Sarwave::DevEnv
