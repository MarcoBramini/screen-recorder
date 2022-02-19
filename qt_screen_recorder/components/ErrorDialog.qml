import QtQuick
import QtQuick.Window
import QtQuick.Controls.Material
import QtQuick.Dialogs

Window {
    id: window
    width: 400
    height: 300
    visible: false
    flags: "FramelessWindowHint"
    color: "transparent"

    property string errorMessage: ""
    Dialog {
        width: 400
        height: 300
        id: errorDialog
        visible: true
        title: "Error"
        standardButtons: Dialog.Close

        contentChildren: Rectangle {
            anchors.fill: parent
            id: errorBody
            Label {
                id: errorLabel
                text: "An unhandled exception has occurred.\nException details:"
            }

            Label {
                anchors.top: errorLabel.bottom
                anchors.topMargin: 16
                anchors.left: errorBody.left
                anchors.right: errorBody.right
                text: errorMessage
                wrapMode: "WordWrap"
                Material.foreground: Material.Grey
            }
        }

        onRejected: {
            main_window.close()
        }
    }
}

/*##^##
Designer {
    D{i:0;formeditorZoom:0.25;height:1080;width:1920}D{i:1}
}
##^##*/

