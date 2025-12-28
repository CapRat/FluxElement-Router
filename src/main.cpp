#include <iostream>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <pipewire/pipewire.h>

#include "PipeWireManager.h"

#include "PipeWireUIModel.h"
int main(int argc, char *argv[])
{

   /* bool start=false;
    PipeWire::PipeWireManager x{[&start]() {
        start=true;
        std::cout<<"PipeWireManager initialized"<<std::endl;
    }};
    while(!start) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(50ms);
    }
    x.connectPorts(66 , 62);
    for (auto n : x.listNodes()) {
        std::cout <<std::to_string(n.id) << ": " << n.name  << "(" <<n.nickname <<") " <<n.mediaClass <<" "<< n.description << std::endl;
        for (auto p : n.ports) {
            std::cout << "    - "<< std::to_string(p.id) <<" " <<p.name << "("<<p.alias<<") " <<p.direction<<std::endl;
        }
    }
    for (auto link:x.listLinks()) {
        std::cout << link.id << ": " << link.outputPort << " ---> " << link.inputPort << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    x.disconnectPorts(66 , 62);*/


    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));

    QGuiApplication app(argc, argv);
    qmlRegisterType<PipeWireUIModel>("PipeWireUiModel", 1, 0, "PipeWireUIModel");
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("RaspiHost", "Main");

   return app.exec();
}
