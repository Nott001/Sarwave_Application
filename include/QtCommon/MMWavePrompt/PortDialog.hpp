#pragma once

#include "MMWave/Porter.hpp"

namespace MMWave::Prompting::Gui {

    // Shows the port-selection dialog, opens the chosen ports, and validates
    // the CLI port against an mmWave device before returning. On failure the
    // dialog stays open with an inline error message. On success the device
    // version is printed into a log pane and a Close button appears to dismiss
    // the dialog. Throws std::runtime_error if the dialog cannot be loaded or
    // the selection is cancelled.
    MMWave::Porter::Context portGUI();

}