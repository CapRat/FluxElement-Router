import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
//import "../PipeWireNode" as PipeWireNode
import PipeWireUiModel 1.0


Page {
    property var nodeElementList: [];
    property var saveLocation: new Map()
    property var startDraggingPoint: null
    property var currentDraggingPoint: null
    id: routingPanel


    function redrawNodesDynamic() {

        // clears nodeElementList, but sore position bevorehand
        for (var i = 0; i < nodeElementList.length; i++) {
            var dynObj = nodeElementList[i]
            if (dynObj.node) {
                saveLocation.set(dynObj.node.id, [dynObj.x, dynObj.y])
            }
            if (dynObj.destroy) {
                dynObj.destroy()
            }

        }
        nodeElementList = []

        // now with everything cleared, redraw everything( but use old positions)
        for (var i = 0; i < model.nodes.length; i++) {
            var node = model.nodes[i]
            var x = i * 20
            var y = 20
            var savedXY = saveLocation.get(node.id)
            if (savedXY) {
                x = savedXY[0]
                y = savedXY[1]
            }
            let dynPWNode = nodeTempalte.createObject(scrollPanel.contentItem, {x: x, y: y, node: node})
            if (dynPWNode) {
                nodeElementList.push(dynPWNode)
            }
        }
    }

    function redrawLinksDynamic() {
        linkCanvas.requestPaint()
    }

    function getPortCoordinate(portId) {
        for (var i = 0; i < nodeElementList.length; i++) {
            let nodeElement = nodeElementList[i]
            if (nodeElement) {
                let point = nodeElement.getPortCoordinate(portId)
                if (point) {
                    point.x = nodeElement.x + point.x
                    point.y = nodeElement.y + point.y
                    return point;
                }
            }
        }
        return null;
    }

    function findPortAt(x, y) {
        for (var i = 0; i < nodeElementList.length; i++) {
            let nodeItem = nodeElementList[i]

            // Node liefert alle Ports (du musst in PipeWireNode eine API machen)
            let ports = nodeItem.portElementList   // z.B. [{id:"in1", x:10, y:20, width:10, height:10}, ...]

            for (var p = 0; p < ports.length; p++) {
                let portElement = ports[p]

                // Port Koordinaten global berechnen
                let absX = nodeItem.x + portElement.x
                let absY = nodeItem.y + portElement.y

                // Port-Größe angenommen 10x10
                if (x >= absX && x <= absX + portElement.width &&
                    y >= absY && y <= absY + portElement.height) {
                    return portElement.port.id
                }
            }
        }
        return null
    }

    PipeWireUIModel {
        id: model
        onNodesChanged: redrawNodesDynamic()

        onLinksChanged: redrawLinksDynamic()
    }
    Flickable {
        id: scrollPanel
        anchors.fill: parent
        contentWidth: parent.width * 2
        contentHeight: parent.height * 2
        // Canvas für Links
        Canvas {
            id: linkCanvas
            anchors.fill: parent

            onWidthChanged: redrawLinksDynamic()
            onHeightChanged: redrawLinksDynamic()

            onPaint: {
                var ctx = getContext("2d")
                ctx.clearRect(0, 0, width, height)

                ctx.strokeStyle = "#FF9800"       // Material Accent Farbe
                ctx.lineWidth = 2
                ctx.lineCap = "round"

                for (var i = 0; i < model.links.length; i++) {
                    var l = model.links[i]

                    var CoordInputPort = getPortCoordinate(l.inputPort)
                    var CoordOutputPort = getPortCoordinate(l.outputPort)
                    if (CoordInputPort && CoordOutputPort) {
                        // Absolute Position der Ports
                        var startX = CoordInputPort.x
                        var startY = CoordInputPort.y
                        var endX = CoordOutputPort.x
                        var endY = CoordOutputPort.y
                        //  console.log(startX+":"+startY+"----->"+endX+":"+endY)

                        // Material-like curved link (Bezier)
                        var cp1X = startX - 100
                        var cp1Y = startY
                        var cp2X = endX + 100
                        var cp2Y = endY

                        ctx.beginPath()
                        ctx.moveTo(startX, startY)

                        ctx.bezierCurveTo(cp1X, cp1Y, cp2X, cp2Y, endX, endY)
                        ctx.stroke()
                    }
                }
                if (routingPanel.startDraggingPoint && routingPanel.currentDraggingPoint) {
                    var startX = routingPanel.startDraggingPoint.x
                    var startY = routingPanel.startDraggingPoint.y
                    var endX = routingPanel.currentDraggingPoint.x
                    var endY = routingPanel.currentDraggingPoint.y
                    var cp1X = startX - 100
                    var cp1Y = startY
                    var cp2X = endX + 100
                    var cp2Y = endY

                    ctx.beginPath()
                    ctx.moveTo(startX, startY)
                    //ctx.lineTo(endX, endY)
                    ctx.bezierCurveTo(cp1X, cp1Y, cp2X, cp2Y, endX, endY)
                    ctx.stroke()
                }
            }
        }
    }
    Component {
        id: nodeTempalte
        PipeWireNode {
            id: nodeX
            onStartDragPort: (portId, portPosX, portPosY) => {
                scrollPanel.interactive = false
                var mapped = scrollPanel.mapToItem(scrollPanel.contentItem, portPosX, portPosY)
                routingPanel.startDraggingPoint = {x: mapped.x, y: mapped.y}
                console.log("Start Dragging" + portId)
                redrawLinksDynamic()
            }
            onDraggingPort: (portId, portPosX, portPosY) => {
                var mapped = scrollPanel.mapToItem(scrollPanel.contentItem, portPosX, portPosY)
                routingPanel.currentDraggingPoint = {x: mapped.x, y: mapped.y}
                if (routingPanel.startDraggingPoint != null)
                    redrawLinksDynamic()
            }
            onDragStopPort: (portId, portPosX, portPosY) => {
                scrollPanel.interactive = true
                routingPanel.startDraggingPoint = null
                routingPanel.currentDraggingPoint = null
                var mapped = scrollPanel.mapToItem(scrollPanel.contentItem, portPosX, portPosY)
                var targetPortId = findPortAt(mapped.x,mapped.y)
                if (targetPortId){
                    model.linkPorts(portId,targetPortId)
                }
                redrawLinksDynamic()
            }
            onDraggingNode: redrawLinksDynamic()
        }
    }


    Component.onCompleted: {
        redrawNodesDynamic()
    }


}