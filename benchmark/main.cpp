#include <QGuiApplication>
#include <QQmlContext>
#include <QQuickView>
#include <QCommandLineParser>
#include <QElapsedTimer>
#include <QTimer>
#include <QRandomGenerator>
#include <cstdio>

#include "StaticServo.h"
#include "QuickServo.h"

static void runPureCppBenchmark()
{
    std::puts("============================================================");
    std::puts("  Pure C++ property-update benchmark (no QML rendering)");
    std::puts("  200 float properties × N iterations");
    std::puts("============================================================\n");

    const QList<int> iterations = {1, 10, 100, 1000, 5000, 10000};

    // Header
    std::printf("  %8s  %12s  %12s  %10s\n", "Iters", "Static (µs)", "Quick (µs)", "Ratio");
    std::printf("  %8s  %12s  %12s  %10s\n", "--------", "------------", "------------", "----------");

    for (int n : iterations) {
        // Benchmark StaticServo
        StaticServo staticServo;
        qint64 staticNs = staticServo.benchmark(n);

        // Benchmark QuickServo
        QuickServo quickServo;
        qint64 quickNs = quickServo.benchmark(n);

        double staticUs = staticNs / 1000.0;
        double quickUs  = quickNs / 1000.0;
        double ratio    = (quickUs > 0) ? staticUs / quickUs : 0;

        std::printf("  %8d  %12.1f  %12.1f  %9.2fx\n", n, staticUs, quickUs, ratio);
    }

    std::puts("\n  (Lower is better. Ratio > 1 means Static is slower.)\n");
}

static void runLiveDemo(QGuiApplication &app, const QString &mode)
{
    QQuickView view;
    view.setResizeMode(QQuickView::SizeRootObjectToView);

    QObject *servo = nullptr;
    QuickServo *quickServo = nullptr;
    StaticServo *staticServo = nullptr;

    if (mode == "quick") {
        quickServo = new QuickServo(&view);
        servo = quickServo;
    } else {
        staticServo = new StaticServo(&view);
        servo = staticServo;
    }

    view.rootContext()->setContextProperty("servo", servo);
    view.rootContext()->setContextProperty("backendName", servo->metaObject()->className());
    view.setSource(QUrl(QStringLiteral("qrc:/benchmark/ServoView.qml")));
    view.setTitle(QStringLiteral("Servo Benchmark — ") + mode);
    view.resize(1200, 800);
    view.show();

    // Simulate a 50 Hz PLC update cycle
    auto *timer = new QTimer(&view);
    float phase = 0.0f;
    QObject::connect(timer, &QTimer::timeout, [&]() {
        float vals[200];
        phase += 0.05f;
        for (int i = 0; i < 200; ++i)
            vals[i] = std::sin(phase + i * 0.1f) * 180.0f;

        QElapsedTimer t;
        t.start();

        if (quickServo)
            quickServo->updateAll(vals);
        else
            staticServo->updateAll(vals);

        qint64 us = t.nsecsElapsed() / 1000;
        // Update an FPS-like metric (property update cost in µs)
        if (quickServo)
            quickServo->insert("servo0", float(us));  // reuse servo0 as timing display
    });
    timer->start(20); // 50 Hz

    app.exec();
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName("Servo Benchmark");

    QCommandLineParser parser;
    parser.setApplicationDescription(
        "Compare StaticServo (200 Q_PROPERTY) vs QuickServo (QuickPropertyMap)\n\n"
        "Modes:\n"
        "  bench        Pure C++ benchmark (no GUI)\n"
        "  live-static  Live demo with StaticServo\n"
        "  live-quick   Live demo with QuickServo");
    parser.addHelpOption();
    parser.addPositionalArgument("mode", "bench, live-static, or live-quick (default: bench)");
    parser.process(app);

    QString mode = parser.positionalArguments().value(0, "bench");

    if (mode == "bench") {
        runPureCppBenchmark();
        return 0;
    } else if (mode == "live-static" || mode == "live-quick") {
        QString backend = mode.mid(5); // "static" or "quick"
        runLiveDemo(app, backend);
        return 0;
    } else {
        std::fprintf(stderr, "Unknown mode: %s\n", qPrintable(mode));
        return 1;
    }
}
