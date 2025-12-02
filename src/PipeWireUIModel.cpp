//
// Created by benji on 29.11.25.
//

#include "PipeWireUIModel.h"


QVariantMap ConvertToQVariantMap(const PipeWire::Port &p) {
    QVariantMap m;
    m["id"] = p.id;
    m["name"] = QString::fromStdString(p.name);
    m["alias"] = QString::fromStdString(p.alias);
    m["direction"] = QString::fromStdString(p.direction);
    return m;
}

QList<QVariant> ConvertToQVariantMap(const std::vector<PipeWire::Port> &p) {
    QList<QVariant> list;
    for (const auto &port: p) {
        list.append(ConvertToQVariantMap(port));
    }
    return list;
}

QVariantMap ConvertToQVariantMap(const PipeWire::Node &n) {
    QVariantMap m;
    m["id"] = n.id;
    m["name"] = QString::fromStdString(n.name);
    m["description"] = QString::fromStdString(n.description);
    m["mediaClass"] = QString::fromStdString(n.mediaClass);
    m["nickname"] = QString::fromStdString(n.nickname);
    m["ports"] = ConvertToQVariantMap(n.ports);
    return m;
}

QList<QVariant> ConvertToQVariantMap(const std::vector<PipeWire::Node> &p) {
    QList<QVariant> list;
    for (const auto &node: p) {
        list.append(ConvertToQVariantMap(node));
    }
    return list;
}

QVariantMap ConvertToQVariantMap(const PipeWire::Link &l) {
    QVariantMap m;
    m["id"] = l.id;
    m["inputPort"] = l.inputPort;
    m["outputPort"] = l.outputPort;
    m["inputNode"] = l.inputNode;
    m["outputNode"] = l.outputNode;
    return m;
}

QList<QVariant> ConvertToQVariantMap(const std::vector<PipeWire::Link> &p) {
    QList<QVariant> list;
    for (const auto &link: p) {
        list.append(ConvertToQVariantMap(link));
    }
    return list;
}

PipeWireUIModel::PipeWireUIModel(QObject *parent) : QObject(parent), manager(
                                                        [this](PipeWire::PipeWireManager::EntityType t) {
                                                            if (t == PipeWire::PipeWireManager::EntityType::Node || t ==
                                                                PipeWire::PipeWireManager::EntityType::Port) {
                                                                emit this->nodesUpdated();
                                                            }
                                                            if (t == PipeWire::PipeWireManager::EntityType::Link) {
                                                                emit this->linksUpdated();
                                                            }
                                                        }) {
}


QList<QVariant> PipeWireUIModel::getNodes() {
    return ConvertToQVariantMap(manager.listNodes());
}

QList<QVariant> PipeWireUIModel::getLinks() {
    return ConvertToQVariantMap(manager.listLinks());
}
