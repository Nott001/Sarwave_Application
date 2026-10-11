// MainWindow.axaml: the window TestRpiApp opens. Its greeting is replaced by the dashboard as
// soon as the window is shown (main.cpp hands it to RunRecorded / RunStuff, as OnOpened does).
import QtQuick
import QtQuick.Controls.Universal

Window {
    id: root

    /// MainViewModel.Greeting
    property string greeting: "Welcome to Avalonia!"

    title: "Sarwave.Source.TestRpiApp"
    // No size is set in MainWindow.axaml; the platform picks one. Three quarters of the screen.
    width: Math.round(Screen.desktopAvailableWidth * 0.75)
    height: Math.round(Screen.desktopAvailableHeight * 0.75)
    visible: true

    TextField {
        anchors.centerIn: parent
        text: root.greeting
        onTextChanged: root.greeting = text
    }
}
