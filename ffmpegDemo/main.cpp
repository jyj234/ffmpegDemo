#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "frameprovider.h"
#include "mux.h"
#include "decoder.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/ffmpegDemo/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
//    qmlRegisterSingletonInstance("FrameProvider",1,0,"FrameProvider",FrameProvider::instance());
    qmlRegisterSingletonInstance("Decoder",1,0,"Decoder",Decoder::instance());

    engine.load(url);

    return app.exec();
}
