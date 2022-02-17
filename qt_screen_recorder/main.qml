import QtQuick
import QtQuick.Window
import QtQuick.Controls.Material
import Backend
import "components"

Window {
    id: main_window
    width: 600
    height: 125

    visible: true
    color: "#00000000"
    flags: "FramelessWindowHint"

    title: "QtScreenRecorder"

    BackEnd {
        id: backend
    }

    ControlPanel {}
}
