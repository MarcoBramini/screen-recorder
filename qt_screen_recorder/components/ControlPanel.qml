import QtQuick
import QtQuick.Controls
import "MaterialDesignIcon"

Rectangle {
    id: controlPanel
    width: 530
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
        height: 25
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
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

    Rectangle {
        id: row
        color: "#00000000"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 19
        anchors.topMargin: 20
        anchors.rightMargin: 25
        anchors.leftMargin: 25

        RoundButton {
            id: recButton
            width: 74
            height: 74
            visible: true
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            hoverEnabled: false
            enabled: false
            anchors.leftMargin: 0
            scale: 1.015
            display: AbstractButton.TextBesideIcon

            indicator: MaterialDesignIcon {
                id: materialDesignIcon
                name: "record"
                visible: false
                anchors.fill: parent
                scale: 0.5
            }

            Connections {
                target: recButton
                function onClicked() {
                    controlPanel.state = "recording"
                    backend.startRecording()
                }
            }

            BusyIndicator {
                id: busyIndicator
                visible: true
                anchors.fill: parent
                scale: 1
                layer.textureSize.width: 1
                anchors.rightMargin: 16
                anchors.leftMargin: 16
                anchors.bottomMargin: 16
                anchors.topMargin: 16
            }

            Label {
                id: recButtonLabel
                visible: false
                text: "00:00:00"
                anchors.top: parent.bottom
                anchors.topMargin: 0
                anchors.horizontalCenter: parent.horizontalCenter
                color: Material.color(Material.Red)

                Item {
                    Timer {
                        interval: 100
                        running: controlPanel.state === "recording"
                        repeat: true
                        onTriggered: {
                            var stats = backend.getRecordingStats()
                            if (Object.keys(stats).length !== 0)
                                recButtonLabel.text = stats["recordingDuration"]
                        }
                    }
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
                function onClicked() {
                    if (controlPanel.state === "recording") {
                        controlPanel.state = "paused"
                        backend.pauseRecording()
                        return
                    }
                    controlPanel.state = "recording"
                    busyIndicator.visible = true
                    backend.resumeRecording()
                    busyIndicator.visible = false
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
                function onClicked() {
                    recButtonLabel.text = "00:00:00"
                    controlPanel.state = "ready"
                    busyIndicator.visible = true
                    backend.stopRecording()
                    busyIndicator.visible = false
                }
            }
        }

        ToolSeparator {
            id: toolSeparator
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: stopButton.right
            anchors.leftMargin: 16
        }

        RoundButton {
            id: configButton
            width: 74
            height: 74
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: toolSeparator.right
            anchors.leftMargin: 16
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
                function onClicked() {
                    (configPanel.visible) ? configPanel.hide(
                                                ) : configPanel.show()
                    controlPanel.state = (controlPanel.state === "config") ? "ready" : "config"
                }
            }
        }

        RoundButton {
            id: quitButton
            width: 74
            height: 74
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: configButton.right
            anchors.leftMargin: 16
            display: AbstractButton.TextBesideIcon
            scale: 1.015
            indicator: MaterialDesignIcon {
                id: materialDesignIcon4
                name: "close"
                visible: true
                anchors.fill: parent
                scale: 0.5
            }
            Connections {
                target: quitButton
                function onClicked() {
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
                visible: true
            }

            PropertyChanges {
                target: recButtonLabel
                x: 20
                y: 70
                visible: true
            }

            PropertyChanges {
                target: busyIndicator
                visible: false
            }

            PropertyChanges {
                target: quitButton
                hoverEnabled: false
                enabled: false
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
                target: materialDesignIcon
                visible: true
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
                visible: true
                hoverEnabled: false
                enabled: false
            }

            PropertyChanges {
                target: busyIndicator
                visible: false
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
                visible: true
            }

            PropertyChanges {
                name: "play"
                target: materialDesignIcon1
            }

            PropertyChanges {
                target: recButtonLabel
                visible: true
            }

            PropertyChanges {
                target: busyIndicator
                visible: false
            }

            PropertyChanges {
                target: quitButton
                hoverEnabled: false
                enabled: false
            }
        },
        State {
            name: "ready"

            PropertyChanges {
                target: recButton
                visible: true
                hoverEnabled: true
                enabled: true
            }

            PropertyChanges {
                target: materialDesignIcon
                visible: true
            }

            PropertyChanges {
                target: busyIndicator
                visible: false
            }
        }
    ]
}
