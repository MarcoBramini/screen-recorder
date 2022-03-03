import QtQuick
import QtQuick.Controls

Window {
    id: window
    x: 0
    y: -1
    width: Screen.width
    height: Screen.height
    color: "#00000000"
    flags: Qt.ToolTip | Qt.FramelessWindowHint | Qt.NoDropShadowWindowHint | Qt.WindowStaysOnTopHint

    onVisibilityChanged: {
        if (window.visible) {
            shortcutWindow.show()
        }
    }

    signal captureRegionSelected(real x, real y, real width, real height)

    function resetCaptureRegion() {
        captureRegion.visible = false
        captureRegion.x = 0
        captureRegion.y = 0
        captureRegion.width = 0
        captureRegion.height = 0
    }

    Window {
        id: shortcutWindow
        visible: true
        color: "#00000000"
        flags: Qt.FramelessWindowHint

        Item {
            anchors.fill: parent
            focus: true

            Shortcut {
                sequence: "Escape"
                onActivated: {
                    // Revert captureRegion
                    var oldCaptureRegion = backend.selectedCaptureRegion
                    if (Object.keys(oldCaptureRegion).length !== 0) {
                        captureRegion.x = oldCaptureRegion.x
                        captureRegion.y = oldCaptureRegion.y
                        captureRegion.width = oldCaptureRegion.width
                        captureRegion.height = oldCaptureRegion.height
                    }

                    window.close()
                    shortcutWindow.close()
                }
            }

            Shortcut {
                sequence: "Return"
                onActivated: {
                    // Do nothing if capture region is empty
                    if (captureRegion.width === 0 || captureRegion.height === 0)
                        return

                    window.captureRegionSelected(parseInt(captureRegion.x),
                                                 parseInt(captureRegion.y),
                                                 parseInt(captureRegion.width),
                                                 parseInt(captureRegion.height))

                    window.close()
                    shortcutWindow.close()
                }
            }
        }
    }

    Rectangle {
        id: container
        color: "#86000000"
        border.width: 0
        anchors.fill: parent

        Rectangle {
            id: labelContainer
            y: 50
            width: 800
            height: 100
            color: "#9b000000"
            radius: 45
            anchors.horizontalCenter: parent.horizontalCenter

            Label {
                id: label
                x: 208
                y: 24
                color: "#ffffff"
                text: qsTr("Draw the capture region with your mouse and press ENTER to confirm.\nPress ESC to exit.")
                anchors.fill: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font.pointSize: 19
            }
        }

        Rectangle {
            id: captureRegion
            visible: false
            color: "#3fffffff"
            border.width: 2
        }

        MouseArea {
            id: screenRegionSelectorMouseArea
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton

            property point clickPos: "1,1"

            onPressed: {
                resetCaptureRegion()
                clickPos = Qt.point(mouseX, mouseY)
                captureRegion.visible = true
                captureRegion.x = clickPos.x
                captureRegion.y = clickPos.y
            }

            onPositionChanged: {
                var delta = Qt.point(mouseX - clickPos.x, mouseY - clickPos.y)
                captureRegion.width = delta.x
                captureRegion.height = delta.y
            }

            onReleased: {
                var delta = Qt.point(mouseX - clickPos.x, mouseY - clickPos.y)
                if (delta.x < 50 || delta.y < 50) {
                    resetCaptureRegion()
                }
            }
        }
    }
}
