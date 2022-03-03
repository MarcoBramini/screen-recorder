import QtQuick
import "MaterialDesignIconGlyphs.js" as MaterialGlyphs

Item {
    property int size: 24
    property string name
    property color color

    width: size
    height: size

    FontLoader {
        id: materialFont
        source: "qrc:assets/fonts/materialdesignicons-webfont.ttf"
    }

    Text {
        anchors.fill: parent

        color: parent.color
        text: MaterialGlyphs.glyphs[parent.name]

        font.family: materialFont.name
        font.pixelSize: parent.height

    }

}

/*##^##
Designer {
    D{i:0;height:24;width:24}D{i:1}D{i:2}
}
##^##*/
