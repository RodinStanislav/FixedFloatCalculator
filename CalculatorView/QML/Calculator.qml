import QtQuick

import CalculatorInputValidator 1.0

Item {
    CalculatorTextInput  {
        id: floatingNumberInput

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 16

        height: 48
        validator: CalculatorInputValidator { id: validator }

        text: "0"
    }

    Text {
        id: errorLabel

        anchors.left: parent.left
        anchors.top: floatingNumberInput.bottom
        anchors.right: parent.right
        anchors.rightMargin: 16

        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        padding: 4

        color: "red"
        font.pixelSize: 16

        text: ''
    }

    ControlPanel {
        anchors.left: parent.left
        anchors.top: errorLabel.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.margins: 16

        onDeletePressed: {
            floatingNumberInput.text = validator.removeSymbol(floatingNumberInput.text)
            errorLabel.text = '';
        }

        onClearPressed: {
            floatingNumberInput.text = "0"
            errorLabel.text = '';
        }

        onInversePressed: {
            floatingNumberInput.text = validator.inverseFirst(floatingNumberInput.text);
            errorLabel.text = '';
        }

        onEvaluatePressed: {
            let evaluetedExpression = calculatorBackend.evaluate(floatingNumberInput.text);

            if (evaluetedExpression !== '') {
                floatingNumberInput.text = evaluetedExpression
            }
        }

        onSymbolPressed: function(symbol) {
            floatingNumberInput.text = validator.appendSymbol(floatingNumberInput.text, symbol)
            errorLabel.text = '';
        }
    }

    Connections {
        target: calculatorBackend
        function onErrorRaised(error) {
            errorLabel.text = error;
        }
    }
}
