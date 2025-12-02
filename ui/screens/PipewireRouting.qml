import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
//import "../PipeWireNode" as PipeWireNode
import PipeWireUiModel 1.0


Page {
    property var createdObjects: [];
    property var saveLocation: new Map()
    id: routingPanel


    function redrawNodesDynamic() {

        // clears createdObjects, but sore position bevorehand
        for (var i = 0; i < createdObjects.length; i++) {
            var dynObj = createdObjects[i]
            var node = dynObj.node
            if (node) {
                saveLocation.set(node.id, [dynObj.x, dynObj.y])
            }
            if (dynObj.destroy) {
                dynObj.destroy()
            }

        }
        createdObjects.createdObjects = []

        // now with everything cleared, redraw everything( but use old positions)
        for (var i = 0; i < model.nodes.length; i++) {
            var node = model.nodes[i]
            var x = i * 20
            var y = 20
            var savedXY=saveLocation.get(node.id)
            if (savedXY) {
                x = savedXY[0]
                y =savedXY[1]
            }
            var inst = nodeTempalte.createObject(routingPanel, {x: x, y: y, node: node})
            if (inst) {
                createdObjects.push(inst)
            }
        }
    }
    Button {
        x:400
        y:200
        text: "rerun"
        onClicked: redrawNodesDynamic()
    }
    PipeWireUIModel {
        id: model
        onNodesChanged: {
            redrawNodesDynamic()
        }
    }

    Component {
        id: nodeTempalte
        PipeWireNode {
            id: nodeX
            onStartDragPort: (portID, portPosX, portPosY) => {
                console.log("Start Dragging" + portID)
            }
        }
    }


    Component.onCompleted: {
        redrawNodesDynamic()
    }


}