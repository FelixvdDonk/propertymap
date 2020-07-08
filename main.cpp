#include <QGuiApplication>
#include "Player.h"

int main(int argc, char *argv[])
{
    QGuiApplication a(argc, argv);

    Player viewer;

    viewer.setTitle("PropertyMap playground");
    viewer.setSource(QUrl("qrc:/Scene.qml"));
    viewer.show();

    return a.exec();
}
