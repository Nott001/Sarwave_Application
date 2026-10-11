#pragma once

#include <QFont>
#include <QString>

namespace Sarwave::DevEnv {

/// Measures how many characters of `text` do not fit in `availableWidth` (the text field's width
/// minus its horizontal padding) when drawn in `font`.
///
/// Returns an integer > 0 for the overflowing character count, or 0 if all characters fit; the
/// whole length when there is no space at all.
int getCharacterOverflow(const QString& text, const QFont& font, double availableWidth);

}  // namespace Sarwave::DevEnv
