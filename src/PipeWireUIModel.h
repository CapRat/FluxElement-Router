//
// Created by benji on 29.11.25.
//

#ifndef RASPIHOST_PIPEWIREUIMODEL_H
#define RASPIHOST_PIPEWIREUIMODEL_H

#include <QVariantMap>
#include <QObject>


#include "PipeWireManager.h"


class PipeWireUIModel :public QObject{
    Q_OBJECT
public:
    PipeWireUIModel(QObject* parent=nullptr);
    Q_PROPERTY(QList<QVariant> nodes READ getNodes NOTIFY nodesUpdated)
    QList<QVariant> getNodes();

signals:
    void nodesUpdated();

private:


    PipeWire::PipeWireManager manager;
};


#endif //RASPIHOST_PIPEWIREUIMODEL_H