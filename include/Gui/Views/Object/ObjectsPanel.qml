// ObjectsPanel.axaml: the tracked targets, one card each, in the order they first appeared.
// As tall as its items until it reaches the height it is given; past that it scrolls.
import QtQuick
import Sarwave.Gui
import QtQuick.Controls.Basic

Rectangle {
    id: root

    required property ObjectsPanelViewModel viewModel

    color: Tokens.surface
    border.color: Tokens.border
    border.width: 1
    radius: 8

    Flickable {
        id: scroller
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        contentWidth: width
        contentHeight: items.implicitHeight
        boundsBehavior: Flickable.StopAtBounds

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Column {
            id: items
            width: scroller.width
            spacing: 8

            Repeater {
                model: root.viewModel

                ObjectItem {
                    required property QtObject item
                    width: items.width
                    viewModel: item
                }
            }
        }
    }
}
