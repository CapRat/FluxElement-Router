#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <pipewire/pipewire.h>

#include "PipeWireManager.h"

#include "PipeWireUIModel.h"
int main(int argc, char *argv[])
{

    /*PipeWire::PipeWireManager x{};
    std::this_thread::sleep_for(std::chrono::milliseconds(900));
    x.connectPorts(72 , 67);
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    x.disconnectPorts(72 , 67);*/
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
