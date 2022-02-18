import QtQuick
import QtQuick.Controls
import "MaterialDesignIcon"

Rectangle {
    id: controlPanel
    width: 600
    height: 125
    visible: true
    color: "#595959"
    radius: 30
    border.width: 0

    ConfigPanel {
        id: configPanel
    }

    MouseArea {
        id: mouseArea
        x: 0
        y: 0
        width: 600
        height: 25
        acceptedButtons: Qt.LeftButton

        property point clickPos: "1,1"

        onPressed: {
            clickPos = Qt.point(mouseX, mouseY)
        }

        onPositionChanged: {
            var delta = Qt.point(mouseX - clickPos.x, mouseY - clickPos.y)
            main_window.x += delta.x
            main_window.y += delta.y
        }

        ToolSeparator {
            id: toolSeparator1
            width: 25
            height: 75
            anchors.verticalCenter: parent.verticalCenter
            anchors.horizontalCenter: parent.horizontalCenter
            rotation: 90
        }
    }

    Row {
        id: row
        width: 494
        height: 85
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        RoundButton {
            id: recButton
            width: 74
            height: 74
            visible: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 0
            scale: 1.015
            display: AbstractButton.TextBesideIcon

            indicator: MaterialDesignIcon {
                id: materialDesignIcon
                name: "record"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }

            Connections {
                target: recButton
                onClicked: {
                    controlPanel.state = "recording"
                    backend.startRecording()
                }
            }
        }
        RoundButton {
            id: pauseButton
            width: 74
            height: 74
            visible: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: recButton.right
            hoverEnabled: false
            enabled: false
            anchors.leftMargin: 16
            scale: 1.015
            display: AbstractButton.TextBesideIcon
            indicator: MaterialDesignIcon {
                id: materialDesignIcon1
                name: "pause"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }

            Connections {
                target: pauseButton
                onClicked: {
                    if (controlPanel.state === "recording") {
                        controlPanel.state = "paused"
                        backend.pauseRecording()
                        return
                    }
                    controlPanel.state = "recording"
                    backend.resumeRecording()
                }
            }
        }

        RoundButton {
            id: stopButton
            width: 74
            height: 74
            visible: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: pauseButton.right
            hoverEnabled: false
            enabled: false
            anchors.leftMargin: 16
            scale: 1.015
            display: AbstractButton.TextBesideIcon
            indicator: MaterialDesignIcon {
                id: materialDesignIcon2
                name: "stop"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }

            Connections {
                target: stopButton
                onClicked: {
                    controlPanel.state = ""
                    backend.stopRecording()
                }
            }
        }

        ToolSeparator {
            id: toolSeparator
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: configButton.left
            anchors.rightMargin: 0
        }

        RoundButton {
            id: configButton
            width: 74
            height: 74
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: quitButton.left
            anchors.rightMargin: 16
            scale: 1.015
            display: AbstractButton.TextBesideIcon
            indicator: MaterialDesignIcon {
                id: materialDesignIcon3
                name: "settings"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }

            Connections {
                target: configButton
                onClicked: {
                    (configPanel.visible) ? configPanel.hide(
                                                ) : configPanel.show()
                    controlPanel.state = (controlPanel.state === "config") ? "" : "config"
                }
            }
        }

        RoundButton {
            id: quitButton
            width: 74
            height: 74
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            display: AbstractButton.TextBesideIcon
            scale: 1.015
            indicator: MaterialDesignIcon {
                id: materialDesignIcon4
                name: "close"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }
            anchors.rightMargin: 0
            Connections {
                target: quitButton
                onClicked: {
                    main_window.close()
                }
            }
        }
    }

    states: [
        State {
            name: "recording"
            PropertyChanges {
                target: pauseButton
                visible: true
                enabled: true
                anchors.leftMargin: 16
            }

            PropertyChanges {
                target: configButton
                visible: true
                hoverEnabled: false
                enabled: false
            }

            PropertyChanges {
                target: stopButton
                visible: true
                enabled: true
                anchors.leftMargin: 16
            }

            PropertyChanges {
                target: recButton
                visible: true
                flat: false
                highlighted: true
                checkable: false
                layer.enabled: false
                focus: false
                autoExclusive: false
                checked: false
                hoverEnabled: false
                enabled: false
                anchors.leftMargin: 0
            }

            PropertyChanges {
                target: materialDesignIcon
                color: Material.color(Material.Red)
            }
        },
        State {
            name: "config"

            PropertyChanges {
                target: configButton
                enabled: true
                highlighted: true
            }

            PropertyChanges {
                target: materialDesignIcon3
                layer.enabled: false
                enabled: true
            }

            PropertyChanges {
                target: toolSeparator
                x: 121
                y: 39
            }

            PropertyChanges {
                target: pauseButton
                hoverEnabled: false
                enabled: false
                checkable: false
            }

            PropertyChanges {
                target: recButton
                hoverEnabled: false
                enabled: false
            }
        },
        State {
            name: "paused"
            PropertyChanges {
                target: pauseButton
                visible: true
                enabled: true
                anchors.leftMargin: 16
            }

            PropertyChanges {
                target: configButton
                visible: true
                hoverEnabled: false
                enabled: false
            }

            PropertyChanges {
                target: stopButton
                visible: true
                enabled: false
                anchors.leftMargin: 16
            }

            PropertyChanges {
                target: recButton
                visible: true
                hoverEnabled: false
                enabled: false
                anchors.leftMargin: 0
            }

            PropertyChanges {
                target: materialDesignIcon
                color: Material.color(Material.Red)
            }

            PropertyChanges {
                name: "play"
                target: materialDesignIcon1
            }
        }
    ]
}
