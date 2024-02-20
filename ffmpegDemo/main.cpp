#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "decoder.h"
#include <QQuickWindow>
#include "videoitem.h"
// #include "dhstream.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/ffmpegDemo/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    // qmlRegisterType<DHStream>("DHStream", 1, 0, "DHStream");
    qmlRegisterType<VideoItem>("VideoItem", 1, 0, "VideoItem");
    // qmlRegisterSingletonInstance("Decoder",1,0,"Decoder",Decoder::instance());
    // qmlRegisterSingletonInstance("DHStream",1,0,"DHStream",DHStream::instance());
    engine.load(url);

    return app.exec();
}
