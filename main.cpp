#include <QGuiApplication>
#include <QCommandLineParser>
#include <QUrl>
#include "Player.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("PropertyMap Playground"));
    app.setApplicationVersion(QStringLiteral("2.0"));

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QStringLiteral("Benchmark different C++-to-QML property map implementations.\n\n"
                       "Implementations:\n"
                       "  static  - Compile-time Q_PROPERTY macros (fastest, not dynamic)\n"
                       "  qml     - QQmlOpenMetaObject-based dynamic properties\n"
                       "  quick   - Custom QMetaObject via QMetaObjectBuilder (default)"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(
        QStringLiteral("mode"),
        QStringLiteral("Property map implementation: static, qml, or quick (default: quick)"),
        QStringLiteral("[mode]"));

    parser.process(app);

    // Determine the map type from the positional argument
    MapType type = MapType::Quick; // default
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        const QString &mode = args.first();
        if (mode.compare(QLatin1String("static"), Qt::CaseInsensitive) == 0)
            type = MapType::Static;
        else if (mode.compare(QLatin1String("qml"), Qt::CaseInsensitive) == 0)
            type = MapType::Qml;
        else if (mode.compare(QLatin1String("quick"), Qt::CaseInsensitive) == 0)
            type = MapType::Quick;
        else {
            qCritical("Unknown mode '%s'. Use: static, qml, or quick.", qPrintable(mode));
            return 1;
        }
    }

    Player viewer(type);
    viewer.setTitle(QStringLiteral("PropertyMap playground"));
    viewer.setSource(QUrl(QStringLiteral("qrc:/Scene.qml")));
    viewer.showFullScreen();

    return app.exec();
}
