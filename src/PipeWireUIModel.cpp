//
// Created by benji on 29.11.25.
//

#include "PipeWireUIModel.h"


QVariantMap ConvertPortToQVariantMap(const PipeWire::Port& p) {
    QVariantMap m;
    m["id"] = p.id;
    m["name"] = QString::fromStdString(p.name);
    m["alias"] = QString::fromStdString(p.alias);
    m["direction"] = QString::fromStdString(p.direction);
    return m;
}

QList<QVariant> ConvertPortsToQVariantMap(const std::vector<PipeWire::Port> &p) {
    QList<QVariant> list;
    for (const auto &port: p) {
        list.append(ConvertPortToQVariantMap(port));
    }
    return list;
}

QVariantMap ConvertNodeToQVariantMap(const PipeWire::Node& n) {
    QVariantMap m;
    m["id"] = n.id;
    m["name"] = QString::fromStdString(n.name);
    m["description"] = QString::fromStdString(n.description);
    m["mediaClass"] = QString::fromStdString(n.mediaClass);
    m["nickname"] = QString::fromStdString(n.nickname);
    m["ports"] = ConvertPortsToQVariantMap(n.ports);
    return m;
}
QList<QVariant> ConvertNodesToQVariantMap(const std::vector<PipeWire::Node> &p) {
    QList<QVariant> list;
    for (const auto &node: p) {
        list.append(ConvertNodeToQVariantMap(node));
    }
    return list;
}

PipeWireUIModel::PipeWireUIModel(QObject *parent) : QObject(parent), manager(
                                                        [this](PipeWire::PipeWireManager::EntityType t) {
                                                            emit this->nodesUpdated();
                                                        }) {
}



QList<QVariant> PipeWireUIModel::getNodes() {
    return ConvertNodesToQVariantMap(manager.listNodes());
}
