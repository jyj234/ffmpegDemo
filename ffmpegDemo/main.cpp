#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "decoder.h"
#include <QQuickWindow>
#include "videoitem.h"
int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);

    QQmlApplicationEngine engine;
    const QUrl url(u"qrc:/ffmpegDemo/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    qmlRegisterType<VideoItem>("VideoItem", 1, 0, "VideoItem");
    engine.load(url);

    return app.exec();
}
