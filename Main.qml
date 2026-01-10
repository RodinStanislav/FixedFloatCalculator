import QtQuick

Window {
    width: 440
    height: 480
    visible: true
    title: qsTr("Calculator")

    color: "beige"

    Calculator {
        anchors.fill: parent
    }
}
