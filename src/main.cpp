#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QUrl>
#include <KLocalizedContext>
#include "drawingarea.h"

Q_DECL_EXPORT int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication app(argc, argv);
    QCoreApplication::setOrganizationName("KDE");
    QCoreApplication::setOrganizationDomain("kde.org");
    QCoreApplication::setApplicationName("KirigamiDraw");

    const char *uri = "org.kde.drawingarea";
    qmlRegisterType<DrawingArea>(uri, 1, 0, "DrawingArea");

    QQmlApplicationEngine engine;

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
