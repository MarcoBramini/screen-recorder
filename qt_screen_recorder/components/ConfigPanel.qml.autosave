import QtQuick
import QtQuick.Controls

import QtQuick.Window

import QtQuick.Layouts
import Qt.labs.platform

Window {
    id: window
    x: main_window.x
    y: main_window.y - 460
    width: main_window.width
    height: 450
    visible: false
    color: "#00000000"
    flags: "FramelessWindowHint"

    ScreenRegionSelector {
        id: screenRegionSelector
        onCaptureRegionSelected: {
            // Propagate capture region change
            var captureRegion = {}
            captureRegion.x = x
            captureRegion.y = y
            captureRegion.width = width
            captureRegion.height = height
            backend.selectedCaptureRegion = captureRegion

            captureRegionSelectButton.update()
            // Update avaialble resolutions
            outputResolutionSelect.model = backend.outputResolutions
        }
    }

    Rectangle {
        id: container
        width: parent.width
        height: parent.height
        visible: true
        color: "#595959"
        radius: 30
        border.color: "#000000"
        border.width: 0
        layer.enabled: false

        FolderDialog {
            id: folderDialog
            folder: backend.outputPath
            onFolderChanged: {
                outputPathValue.text = folder
                backend.outputPath = folder
            }
        }

        Column {
            id: column
            y: 101
            visible: true
            anchors.fill: parent
            anchors.rightMargin: 16
            anchors.leftMargin: 16

            Row {
                id: row
                height: 61
                visible: true
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 4
                anchors.rightMargin: 0
                anchors.leftMargin: 0

                Label {
                    id: outputPathLabel
                    visible: true
                    text: qsTr("Output Path")
                    anchors.verticalCenter: parent.verticalCenter
                    Material.foreground: "white"
                }

                Label {
                    id: outputPathValue
                    width: 300
                    visible: true
                    text: qsTr("C:/Users/...")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: outputPathButton.left
                    horizontalAlignment: Text.AlignRight
                    wrapMode: Text.NoWrap
                    anchors.rightMargin: 16
                    elide: Text.ElideLeft
                    Material.foreground: "white"
                }

                Button {
                    id: outputPathButton
                    width: 116
                    text: qsTr("Browse")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 0

                    Connections {
                        target: outputPathButton
                        onClicked: {
                            folderDialog.visible = true
                        }
                    }
                }
            }

            Row {
                id: row1
                height: 61
                visible: true
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row.bottom
                Label {
                    id: videoInputDeviceLabel
                    text: qsTr("Video Input Device")
                    anchors.verticalCenter: parent.verticalCenter
                    Material.foreground: "white"
                }

                ComboBox {
                    id: videoInputDeviceSelect
                    width: 250
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    model: backend.videoDevices
                    onCurrentIndexChanged: {
                        console.log(currentIndex)
                        backend.selectedVideoDeviceIndex = currentIndex
                    }
                }
                anchors.topMargin: 16
                anchors.rightMargin: 0
                anchors.leftMargin: 0
            }

            Row {
                id: row2
                height: 61
                visible: true
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row1.bottom
                Label {
                    id: audioInputDeviceLabel
                    text: qsTr("Audio Input Device")
                    anchors.verticalCenter: parent.verticalCenter
                    Material.foreground: "white"
                }

                ComboBox {
                    id: audioInputDeviceSelect
                    width: 250
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    model: backend.audioDevices
                    onCurrentIndexChanged: {
                        console.log(currentIndex)
                        backend.selectedAudioDeviceIndex = currentIndex
                    }
                }
                anchors.topMargin: 16
                anchors.leftMargin: 0
                anchors.rightMargin: 0
            }

            Row {
                id: row3
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row2.bottom
                anchors.topMargin: 16
                anchors.rightMargin: 0
                anchors.leftMargin: 0

                Label {
                    id: outputResolutionLabel
                    text: qsTr("Output Resolution")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    Material.foreground: "white"
                }

                ComboBox {
                    id: outputResolutionSelect
                    width: 250
                    height: 48
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    anchors.rightMargin: 0
                    model: backend.outputResolutions
                    onCurrentIndexChanged: {
                        console.log(currentIndex)
                        backend.selectedOutputResolutionIndex = currentIndex
                    }
                }
            }

            Row {
                id: row4
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row3.bottom
                Label {
                    id: captureRegionLabel
                    text: qsTr("Capture Region")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    Material.foreground: "white"
                }

                Button {
                    id: captureRegionSelectButton
                    width: 116
                    text: {
                        if (Object.keys(
                                    backend.selectedCaptureRegion).length !== 0)
                            return "UPDATE"
                        return qsTr("SELECT")
                    }
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: captureRegionResetButton.left
                    anchors.rightMargin: 16

                    Connections {
                        target: captureRegionSelectButton
                        onClicked: screenRegionSelector.show()
                    }
                }

                Button {
                    id: captureRegionResetButton
                    width: 116
                    text: qsTr("RESET")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    hoverEnabled: true
                    enabled: false
                    autoRepeat: false
                    autoExclusive: false
                    checked: false
                    checkable: false
                    highlighted: false
                    flat: false
                    anchors.rightMargin: 0
                }
                anchors.topMargin: 16
                anchors.leftMargin: 0
                anchors.rightMargin: 0
            }

            Row {
                id: row5
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row4.bottom
                Label {
                    id: framerateLabel
                    text: qsTr("Framerate")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.leftMargin: 0
                    Material.foreground: "white"
                }

                ComboBox {
                    id: framerateSelect
                    width: 250
                    height: 48
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    model: ["30", "24", "20", "60"]
                    anchors.rightMargin: 0
                }
                anchors.topMargin: 16
                anchors.leftMargin: 0
                anchors.rightMargin: 0
            }
        }
    }
}
