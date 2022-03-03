import QtQuick
import QtQuick.Controls
import "MaterialDesignIcon"

Window {
    id: window
    width: 450
    height: 300
    visible: true
    flags: "FramelessWindowHint"
    color: "#595959"

    property var permissionsStatus: ({})

    Label {
        id: titleLabel
        x: 13
        y: 16
        width: 188
        height: 37
        text: qsTr("Permissions Setup")
        font.pointSize: 16
        Material.foreground: "white"
    }

    Label {
        id: subtitleLabel
        x: 13
        y: 44
        width: 374
        height: 49
        text: qsTr("This application requires access to the following resources.\nClick on the buttons below to allow the usage of capture devices.")
        Material.foreground: "white"
    }

    RoundButton {
        id: allowCameraUsageButton
        x: 88
        y: 93
        width: 275
        height: 50
        display: AbstractButton.TextBesideIcon
        indicator: MaterialDesignIcon {
            id: cameraMDI
            name: "camera"
            width: 40
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: 16
            anchors.bottomMargin: 4
            anchors.topMargin: 4
            scale: 0.5
        }

        Connections {
            target: allowCameraUsageButton
            function onClicked() {
                backend.setupCameraUsagePermission()
            }
        }

        Label {
            id: allowCameraUsageLabel
            x: 62
            y: 17
            text: qsTr("Allow camera usage")
        }

        MaterialDesignIcon {
            id: allowCameraUsageStatus
            name: permissionsStatus["cameraCaptureAllowed"] ? "check-circle" : "close-circle"
            x: 227
            width: 40
            color: Material.color(
                       permissionsStatus["cameraCaptureAllowed"] ? Material.Green : Material.Red)
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            scale: 0.5
            anchors.bottomMargin: 4
            anchors.topMargin: 4
            anchors.rightMargin: 16
        }
    }

    RoundButton {
        id: allowMicrophoneUsageButton
        x: 88
        y: 143
        width: 275
        height: 50
        indicator: MaterialDesignIcon {
            id: cameraMDI1
            name: "microphone"
            width: 40
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            scale: 0.5
            anchors.bottomMargin: 4
            anchors.leftMargin: 16
            anchors.topMargin: 4
        }
        display: AbstractButton.TextBesideIcon
        Connections {
            target: allowMicrophoneUsageButton
            function onClicked() {
                backend.setupMicrophoneUsagePermission()
            }
        }

        Label {
            id: allowMicrophoneUsageLabel
            x: 62
            y: 17
            text: qsTr("Allow microphone usage")
        }

        MaterialDesignIcon {
            id: allowMicrophoneUsageStatus
            name: permissionsStatus["microphoneCaptureAllowed"] ? "check-circle" : "close-circle"
            x: 227
            width: 40
            color: Material.color(
                       permissionsStatus["microphoneCaptureAllowed"] ? Material.Green : Material.Red)
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            scale: 0.5
            anchors.bottomMargin: 4
            anchors.rightMargin: 16
            anchors.topMargin: 4
        }
    }

    RoundButton {
        id: allowScreenCaptureButton
        x: 88
        y: 193
        width: 275
        height: 50
        indicator: MaterialDesignIcon {
            id: cameraMDI2
            name: "monitor"
            width: 40
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            scale: 0.5
            anchors.bottomMargin: 4
            anchors.leftMargin: 16
            anchors.topMargin: 4
        }
        display: AbstractButton.TextBesideIcon
        Connections {
            target: allowScreenCaptureButton
            function onClicked() {
                backend.setupScreenCapturePermission()
            }
        }

        Label {
            id: allowScreenCaptureLabel
            x: 62
            y: 17
            text: qsTr("Allow screen capture")
        }

        MaterialDesignIcon {
            id: allowScreenCaptureStatus
            name: permissionsStatus["screenCaptureAllowed"] ? "check-circle" : "close-circle"
            x: 227
            width: 40
            color: Material.color(
                       permissionsStatus["screenCaptureAllowed"] ? Material.Green : Material.Red)
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            scale: 0.5
            anchors.bottomMargin: 4
            anchors.rightMargin: 16
            anchors.topMargin: 4
        }
    }

    Button {
        id: restartButton
        x: 234
        y: 251
        width: 100
        height: 45
        text: qsTr("restart")

        Connections {
            target: restartButton
            function onClicked() {
                backend.restartApp()
            }
        }
    }

    Button {
        id: exitButton
        x: 340
        y: 251
        width: 100
        height: 45
        text: qsTr("close")

        Connections {
            target: exitButton
            function onClicked() {
                Qt.quit()
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:1.1}D{i:1}D{i:2}D{i:5}D{i:6}D{i:7}D{i:4}D{i:3}D{i:10}D{i:11}
D{i:12}D{i:8}D{i:15}D{i:16}D{i:17}D{i:13}D{i:19}D{i:18}D{i:21}D{i:20}
}
##^##*/

