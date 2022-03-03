import QtQuick
import QtQuick.Window
import QtQuick.Controls.Material
import Backend
import "components"

Window {
    id: main_window
    width: 530
    height: 125

    visible: true
    color: "#00000000"

    flags: "FramelessWindowHint"
    title: "QtScreenRecorder"

    BackEnd {
        Component.onCompleted: function () {
            if (backend.showPermissionsSetup()) {
                permissionsSetupPanel.visible = true
                permissionsSetupPanel.permissionsStatus = backend.permissionsStatus
                return
            }
            controlPanel.visible = true
        }

        id: backend
        onErrorMessageChanged: {
            if (backend.errorMessage) {
                errorDialog.errorMessage = backend.errorMessage
                errorDialog.visible = true
            }
        }

        onPermissionsStatusChanged: {
            permissionsSetupPanel.permissionsStatus = backend.permissionsStatus
        }
    }

    PermissionsSetupPanel {
        id: permissionsSetupPanel
        visible: false
    }

    ControlPanel {
        id: controlPanel
        state: "ready"
        visible: false
    }

    ErrorDialog {
        id: errorDialog
    }
}
