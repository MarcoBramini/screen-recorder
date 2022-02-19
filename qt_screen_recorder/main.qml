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
        id: backend
        onErrorMessageChanged: {
            if (backend.errorMessage) {
                errorDialog.errorMessage = backend.errorMessage
                errorDialog.visible = true
            }
        }


        /*onIsReadyChanged: {
            if (backend.isReady) {
                controlPanel.state = "ready"
            }
        }*/
    }


    /*onVisibleChanged: {
        if (!backend.isReady) {
            backend.init()
        }
    }*/
    ControlPanel {
        id: controlPanel
        state: "ready"
    }

    ErrorDialog {
        id: errorDialog
    }
}
