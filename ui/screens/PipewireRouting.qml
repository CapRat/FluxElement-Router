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

    function getPortCoordinateAndDir(portId) {
        for (var i = 0; i < nodeElementList.length; i++) {
            let nodeElement = nodeElementList[i]
            if (nodeElement) {
                let ret = nodeElement.getPortCoordinateAndDirection(portId)

                if (ret) {
                    ret.color=nodeElement.mapTypeToColor()
                    ret.x = nodeElement.x + ret.x
                    ret.y = nodeElement.y + ret.y
                    return ret;
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
        onNodesChanged: {
            redrawNodesDynamic()
            redrawLinksDynamic()
        }

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
                function drawBezier2(startX, startY, endX, endY, fromIsOutput, toIsOutput) {
                    let dx = Math.max(40, Math.abs(endX - startX) * 0.35)
                    let cp1X
                    let cp1Y = startY
                    let cp2X
                    let cp2Y = endY
                    // Ausgang je nach Startport
                    if (fromIsOutput)
                        cp1X = startX + dx
                    else
                        cp1X = startX - dx
                    // Eingang je nach Zielport
                    if (toIsOutput)
                        cp2X = endX + dx
                    else
                        cp2X = endX - dx
                    ctx.beginPath()
                    ctx.moveTo(startX, startY)
                    ctx.bezierCurveTo(cp1X, cp1Y, cp2X, cp2Y, endX, endY)
                    ctx.stroke()
                }

                for (var i = 0; i < model.links.length; i++) {
                    var l = model.links[i]
                    var coordInputPort = getPortCoordinateAndDir(l.inputPort)
                    var coordOutputPort = getPortCoordinateAndDir(l.outputPort)
                    ctx.strokeStyle =coordInputPort.color
                    if (coordInputPort && coordOutputPort) {
                        drawBezier2(coordInputPort.x, coordInputPort.y, coordOutputPort.x, coordOutputPort.y, coordInputPort.direction === "out", coordOutputPort.direction === "out")
                    }
                }
                if (routingPanel.startDraggingPoint && routingPanel.currentDraggingPoint) {
                    var startCoord = getPortCoordinateAndDir(routingPanel.startDraggingPoint.id)
                    ctx.strokeStyle =startCoord.color
                    var targetX= routingPanel.currentDraggingPoint.x
                    var targetY=routingPanel.currentDraggingPoint.y
                    var targetPortId = findPortAt(targetX, targetY)
                    var targetPort = getPortCoordinateAndDir(targetPortId)
                    if(targetPort){
                        targetX=targetPort.x
                        targetY=targetPort.y
                    }
                    drawBezier2(startCoord.x, startCoord.y, targetX, targetY, startCoord.direction === "out", routingPanel.currentDraggingPoint.x < startCoord.x)
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
                routingPanel.startDraggingPoint = {x: mapped.x, y: mapped.y, id: portId}
                redrawLinksDynamic()
            }
            onDraggingPort: (portId, portPosX, portPosY) => {
                var mapped = scrollPanel.mapToItem(scrollPanel.contentItem, portPosX, portPosY)
                routingPanel.currentDraggingPoint = {x: mapped.x, y: mapped.y, id: findPortAt(mapped.x, mapped.y)}
                if (routingPanel.startDraggingPoint != null)
                    redrawLinksDynamic()
            }
            onDragStopPort: (portId, portPosX, portPosY) => {
                scrollPanel.interactive = true
                routingPanel.startDraggingPoint = null
                routingPanel.currentDraggingPoint = null
                var mapped = scrollPanel.mapToItem(scrollPanel.contentItem, portPosX, portPosY)
                var targetPortId = findPortAt(mapped.x, mapped.y)
                if (targetPortId) {
                    model.linkPorts(portId, targetPortId)
                }
                redrawLinksDynamic()
            }
            onDraggingNode: redrawLinksDynamic()
        }
    }
    function autoLayoutNodes() {
        console.log("AUTO LAYOUT START")

        if (nodeElementList.length === 0)
            return

        //
        // 1) Graph analysieren (Incoming/Outgoing)
        //
        let incoming = new Map()
        let outgoing = new Map()

        for (let n of nodeElementList) {
            incoming.set(n.node.id, new Set())
            outgoing.set(n.node.id, new Set())
        }

        for (let link of model.links) {
            let outNode = null
            let inNode = null

            for (let n of nodeElementList) {
                if (n.portElementList.some(p => p.port.id === link.outputPort))
                    outNode = n.node.id
                if (n.portElementList.some(p => p.port.id === link.inputPort))
                    inNode = n.node.id
            }
            if (outNode && inNode) {
                outgoing.get(outNode).add(inNode)
                incoming.get(inNode).add(outNode)
            }
        }

        //
        // 2) Node-Level bestimmen (graph layering)
        //
        let level = new Map()
        let queue = []

        // Startnodes zuerst (keine incoming edges)
        for (let [id, inc] of incoming.entries()) {
            if (inc.size === 0) {
                level.set(id, 0)
                queue.push(id)
            }
        }

        // BFS layering
        while (queue.length > 0) {
            let id = queue.shift()
            let lvl = level.get(id)
            for (let child of outgoing.get(id)) {
                if (!level.has(child)) {
                    level.set(child, lvl + 1)
                    queue.push(child)
                }
            }
        }

        // Unverbundene Nodes → Level 0
        for (let n of nodeElementList)
            if (!level.has(n.node.id))
                level.set(n.node.id, 0)

        //
        // 3) Nodes pro Level sammeln
        //
        let levelMap = new Map()
        for (let [id, lvl] of level.entries()) {
            if (!levelMap.has(lvl)) levelMap.set(lvl, [])
            levelMap.get(lvl).push(id)
        }

        function byId(id) {
            return nodeElementList.find(n => n.node.id === id)
        }

        //
        // 4) Platzieren — unendlich Platz erlaubt
        //    → kein Überlappen, egal wie groß
        //
        let columnSpacing = 300    // viel Platz horizontal
        let rowSpacing = 40        // vertikal
        let startX = 40
        let startY = 40

        for (let [lvl, ids] of levelMap.entries()) {

            let x = startX + lvl * columnSpacing

            // deterministisch sortieren:
            ids.sort()  // Alphabetisch oder nach ID

            // wir wissen: wir dürfen beliebig weit runter gehen
            let currentY = startY

            for (let id of ids) {
                let n = byId(id)

                // garantiere, dass es nicht überlappt
                n.x = x
                n.y = currentY

                // nächster Node in dieser Spalte kommt einfach weiter unten
                currentY += n.height + rowSpacing
            }
        }

        // Canvas updaten
        redrawLinksDynamic()
        console.log("AUTO LAYOUT DONE")
    }



    Timer {
        interval: 500     // 1 second
        repeat: false
        running: true      // starts automatically

        onTriggered: {
            autoLayoutNodes()

        }
    }

    Component.onCompleted: {
        redrawNodesDynamic()
    }


}