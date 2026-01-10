import QtQuick
import QtQuick.Controls.Basic

Button {
    id: button

    font.pixelSize: 20
    font.bold: true

    background: Rectangle {
        border.width: 1
        border.color: "black"

        color: !button.enabled ? "whitesmoke" :
                button.pressed ? "paleturquoise" :
                button.hovered ? "lightcyan" :
                                 "whitesmoke"
    }
}
