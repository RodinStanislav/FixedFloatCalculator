import QtQuick
import QtQuick.Layouts

Item {
    id: panel
    readonly property int cellSizeWidth: width / 5
    readonly property int cellSizeHeight: height / 5

    signal deletePressed
    signal clearPressed
    signal evaluatePressed
    signal inversePressed
    signal symbolPressed(string symbol)

    RowLayout {
        id: funcsLayout

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        spacing: 0

        CalculatorButton {
            Layout.preferredWidth: panel.cellSizeWidth
            Layout.preferredHeight: panel.cellSizeHeight

            text: "Delete"

            onClicked: {
                panel.deletePressed()
            }
        }

        CalculatorButton {
            Layout.preferredWidth: panel.cellSizeWidth
            Layout.preferredHeight: panel.cellSizeHeight

            text: "Clear"

            onClicked: {
                panel.clearPressed()
            }
        }

        CalculatorButton {
            Layout.preferredWidth: panel.cellSizeWidth
            Layout.preferredHeight: panel.cellSizeHeight

            text: "+/-"

            onClicked: {
                panel.inversePressed()
            }
        }

        CalculatorButton {
            Layout.preferredWidth: panel.cellSizeWidth
            Layout.preferredHeight: panel.cellSizeHeight

            text: "."

            onClicked: {
                panel.symbolPressed(text)
            }
        }

        CalculatorButton {
            Layout.preferredWidth: panel.cellSizeWidth
            Layout.preferredHeight: panel.cellSizeHeight

            text: "="

            onClicked: {
                panel.evaluatePressed()
            }
        }
    }

    GridLayout {
        columns: 5
        rows: 4

        columnSpacing: 0
        rowSpacing: 0

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: funcsLayout.top

        Repeater {
            model: ListModel {}

            delegate: CalculatorButton {
                Layout.preferredWidth: panel.cellSizeWidth
                Layout.preferredHeight: panel.cellSizeHeight

                Layout.row: row
                Layout.column: column

                text: symbol

                onClicked: {
                    panel.symbolPressed(symbol)
                }
            }

            Component.onCompleted: {
                let hexSymbols = ['0', '1', '2', '3',
                                  '4', '5', '6', '7',
                                  '8', '9', 'A', 'B',
                                  'C', 'D', 'E', 'F']

                for (let i = 0; i < hexSymbols.length; i++) {
                    model.append({
                        symbol: hexSymbols[i],
                        row: i / 4,
                        column: i % 4
                    })
                }

                let operations = ['+', '-', '*', '/']

                for (let i = 0; i < operations.length; i++) {
                    model.append({
                        symbol: operations[i],
                        row: i,
                        column: 4
                    })
                }
            }
        }
    }
}
