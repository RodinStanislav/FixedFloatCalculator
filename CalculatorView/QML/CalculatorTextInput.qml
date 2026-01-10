import QtQuick
import QtQuick.Controls.Basic

TextInput  {
    id: floatingNumberInput

    font.pixelSize: 32
    font.bold: true
    horizontalAlignment: TextInput.AlignRight
    verticalAlignment: TextInput.AlignVCenter

    rightPadding: 8
    leftPadding: 8

    clip: true

    Rectangle {
        anchors.fill: parent
        color: "whitesmoke"
        border.width: 1
        border.color: "black"
        z: parent.z - 1
    }
}
