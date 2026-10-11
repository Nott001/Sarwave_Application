#pragma once

namespace Sarwave::DevEnv {

/// Call once after the QGuiApplication exists (and after Sarwave::Core::Gui::Views::initialize()):
/// registers the dialogue types of the Sarwave.DevEnv QML module.
void initialize();

}  // namespace Sarwave::DevEnv
