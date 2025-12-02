import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls.Material

Item {
    id: pipeWireNodeRoot
    property var node
    property int portHeight: 20 // height of a port
    property int portYDistance: 5 // how far away the ports are
    property int initialportYDistance: 25 // how large is the initial distance to the first port
    property int portXDistance: 5 // offset how far the ports are inside the node
    property var portElementList:[]
    function getMaxLengthPorts() {
        var inCounter = 0
        var outCounter = 0
        for (let i = 0; i < node.ports.length; i++) {
            var p = node.ports[i]
            if (p.direction === "in") {
                inCounter++
            }
            if (p.direction === "out") {
                outCounter++
            }
        }
        return Math.max(inCounter, outCounter)
    }

    function getPortCoordinate(portId){
        for(var i =0; i< portElementList.length;i++){
            let pElement=portElementList[i]
            if(pElement && pElement.port.id===portId)
            {
                //return pElement.mapToItem(null,pElement.x-(pElement.width/2), pElement.y-50)
                if(pElement.port.direction==="in") {
                    return {x: pElement.x, y: pElement.y+pElement.height/2}
                }
                else{
                    return {x: pElement.x+pElement.width, y: pElement.y+pElement.height/2}
                }
            }
        }
        return null
    }


    Component {
        id: portTemplate

        Item {
            property var port;
            property var self;
            width:port1.width
            height:port1.height
            Rectangle {
                id: port1
                width: portText.width
                height: portHeight
                radius: 2
                color: Material.accent
                x: pipeWireNode.x + (port.direction === "in" ? -portXDistance : pipeWireNode.width - port1.width + portXDistance)
                y: pipeWireNode.y + portYDistance
                Text {
                    id: portText
                    font.pixelSize: 12
                    text: port.name
                    elide: Text.ElideLeft
                    width: implicitWidth < 65 ? implicitWidth : 65
                    color: Material.foreground
                }

                MouseArea {
                    id: portDragArea
                    anchors.fill: port1

                    // drag.target: port1
                    onPressed: (mouse) => {
                        var globalPos=portDragArea.mapToGlobal(mouse.x,mouse.y)
                        startDragPort(port.id, globalPos.x, globalPos.y)
                    }

                    onPositionChanged: (mouse) => {
                        var globalPos=portDragArea.mapToGlobal(mouse.x,mouse.y)
                        draggingPort(port.id, globalPos.x, globalPos.y)
                    }

                    onReleased: (mouse) => {
                        var globalPos=portDragArea.mapToGlobal(mouse.x,mouse.y)
                        dragStopPort(port.id, globalPos.x, globalPos.y)
                    }
                }
            }
        }
    }

    signal startDragPort(int portId, int portPosX, int portPosY)

    signal draggingPort(int portId, int curX, int curY)

    signal dragStopPort(int portId, int curX, int curY)

    signal draggingNode()

    Rectangle {
        id: pipeWireNode
        width: nodeName.width + 20
        height: getMaxLengthPorts() * (portHeight + portYDistance) + initialportYDistance + portYDistance
        color: Material.primary
        radius: 5
        border.color: Material.accent
        Text {
            id: nodeName
            elide: Text.ElideRight
            width: implicitWidth < 120 ? implicitWidth : 120
            x: 10
            y: 10
            color: Material.foreground
            text: node.name

        }
    }
    MouseArea {
        id: dragArea
        anchors.fill: pipeWireNode
        drag.target: pipeWireNodeRoot
        onPositionChanged: {
            if (drag.active) {
                draggingNode()
            }
        }

    }
    Component.onCompleted: {
        let inCounter = 0;
        let outCounter = 0
        for (let i = 0; i < node.ports.length; i++) {
            let p = node.ports[i]
            let inst=null
            if (p.direction === "in") {
                inst=portTemplate.createObject(pipeWireNodeRoot, {
                    y: initialportYDistance + inCounter * portYDistance + inCounter * portHeight,
                    port: p
                })

                inCounter++
            } else {
                inst=portTemplate.createObject(pipeWireNodeRoot, {
                    y: initialportYDistance + outCounter * portYDistance + outCounter * portHeight,
                    port: p
                })
                outCounter++
            }
            inst.self=inst
            if(inst){
                portElementList.push(inst)
            }
        }
    }
}