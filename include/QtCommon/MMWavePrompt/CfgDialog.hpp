#pragma once

#include "MMWave/Porter.hpp"

#include <string>

namespace MMWave::Prompting::Gui {

    // Shows the configuration-file selection dialog, restricted to *.cfg
    // files, and returns the chosen path on success. On failure the dialog
    // stays open with an inline error message. Throws std::runtime_error if
    // the dialog cannot be loaded or the selection is cancelled.
    std::string cfgGUI();

    // Like cfgGUI but also streams the chosen config to the device via
    // MMWave::Porter::sendConfigFile. Each command/response is echoed into a
    // scrollable output pane, and the Close button only appears once the send
    // has completed; any error falls back to the normal in-dialog workflow.
    // The optional path pre-fills the selection with a config found ahead of
    // time; if it proves invalid, the dialog behaves as usual. Throws
    // std::runtime_error if the dialog cannot be loaded or the selection is
    // cancelled.
    std::string cfgGUIAndSend(MMWave::Porter::Context &ctx,
                              const std::string &path = {});
}