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

    Rectangle {
        id: container
        width: parent.width
        height: parent.height
        color: "#595959"
        radius: 30
        border.width: 0
        layer.enabled: false

        Column {
            id: column
            anchors.fill: parent
            anchors.rightMargin: 16
            anchors.leftMargin: 16

            Row {
                id: row
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.topMargin: 4

                Label {
                    id: outputPathLabel
                    text: "Output Path"
                    anchors.verticalCenter: parent.verticalCenter
                    Material.foreground: "white"
                }

                Label {
                    id: outputPathValue
                    width: 300
                    text: backend.outputPath
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
                    text: "Browse"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    Connections {
                        target: outputPathButton
                        onClicked: {
                            folderDialog.visible = true
                        }
                    }
                }

                FolderDialog {
                    id: folderDialog
                    folder: backend.outputPath
                    onFolderChanged: {
                        backend.outputPath = folder
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

                    model: backend.videoDevices
                    currentIndex: backend.selectedVideoDeviceIndex
                    onCurrentIndexChanged: {
                        backend.selectedVideoDeviceIndex = currentIndex
                    }
                }
                anchors.topMargin: 16
            }

            Row {
                id: row2
                height: 61
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

                    model: backend.audioDevices
                    currentIndex: backend.selectedAudioDeviceIndex
                    onCurrentIndexChanged: {
                        backend.selectedAudioDeviceIndex = currentIndex
                    }
                }
                anchors.topMargin: 16
            }

            Row {
                id: row3
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row2.bottom
                anchors.topMargin: 16

                Label {
                    id: outputResolutionLabel
                    text: qsTr("Output Resolution")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    Material.foreground: "white"
                }

                ComboBox {
                    id: outputResolutionSelect
                    width: 250
                    height: 48
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    model: backend.outputResolutions
                    currentIndex: backend.selectedOutputResolutionIndex
                    onCurrentIndexChanged: {
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
                anchors.topMargin: 16

                Label {
                    id: captureRegionLabel
                    text: qsTr("Capture Region")
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    Material.foreground: "white"
                }

                Button {
                    id: captureRegionSelectButton
                    width: 116
                    text: "SELECT"

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
                    text: "RESET"

                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right

                    enabled: false

                    Connections {
                        target: captureRegionResetButton
                        onClicked: function () {
                            captureRegionSelectButton.text = "SELECT"
                            captureRegionResetButton.enabled = false
                            backend.resetCaptureRegion()
                            screenRegionSelector.resetCaptureRegion()
                            outputResolutionSelect.model = backend.outputResolutions
                        }
                    }
                }

                ScreenRegionSelector {
                    id: screenRegionSelector
                    onCaptureRegionSelected: function (x, y, width, height) {
                        // Propagate capture region change
                        var captureRegion = {}
                        captureRegion.x = x
                        captureRegion.y = y
                        captureRegion.width = width
                        captureRegion.height = height
                        backend.selectedCaptureRegion = captureRegion

                        // Update widgets
                        outputResolutionSelect.model = backend.outputResolutions
                        captureRegionSelectButton.text = "UPDATE"
                        captureRegionResetButton.enabled = true
                    }
                }
            }

            Row {
                id: row5
                height: 61
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: row4.bottom
                Label {
                    id: framerateLabel
                    text: "Framerate"
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    Material.foreground: "white"
                }

                ComboBox {
                    id: framerateSelect
                    width: 250
                    height: 48
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.right: parent.right
                    model: backend.framerates
                    currentIndex: backend.selectedFramerateIndex
                    onCurrentIndexChanged: {
                        backend.selectedFramerateIndex = currentIndex
                    }
                }
                anchors.topMargin: 16
            }
        }
    }
}
