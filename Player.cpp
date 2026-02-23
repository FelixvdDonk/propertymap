#include "Player.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QtMath>
#include <QQmlContext>
#include <QRandomGenerator>

#include "QmlPropertyMap/QmlPropertyMap.h"
#include "QuickPropertyMap/QuickPropertyMap.h"
#include "StaticPropertyMap/StaticPropertyMap.h"

namespace {

constexpr int COUNT = 1000;
constexpr double R = 6;
constexpr double DEG2RAD = M_PI / 180.0;

double posX(int i, double angle) { return (i / 3) * R * cos(angle * DEG2RAD) + 10; }
double posY(int i, double angle) { return (i / 3) * R * sin(angle * DEG2RAD) + 10; }

using PairData = QList<QPair<QByteArray, QVariant>>;

void initStatic(StaticPropertyMap &pm, const PairData &data)
{
    for (const auto &i : data)
        pm.insert(i.first, i.second);
}

void initQml(QmlPropertyMap &pm, const PairData &data)
{
    for (const auto &i : data)
        pm.insert(i.first, i.second);
}

void initQuick(QuickPropertyMap &pm, const PairData &data)
{
    for (const auto &i : data)
        pm.addProperty(i.first, i.second);
    pm.build();
}

} // anonymous namespace

Player::Player(MapType type, QWindow *parent)
    : QQuickView(parent)
    , m_type(type)
    , m_timer(new QTimer(this))
    , m_step(0)
{
    // Create the chosen backend
    switch (m_type) {
    case MapType::Static: {
        auto *p = new StaticPropertyMap(this);
        m_staticMap = p;
        m_propertyMap = p;
        break;
    }
    case MapType::Qml: {
        auto *p = new QmlPropertyMap(this);
        m_qmlMap = p;
        m_propertyMap = p;
        break;
    }
    case MapType::Quick: {
        auto *p = new QuickPropertyMap(this);
        m_quickMap = p;
        m_propertyMap = p;
        break;
    }
    }

    setResizeMode(QQuickView::SizeRootObjectToView);

    m_data.resize(3 * COUNT);
    m_speed.resize(COUNT);

    auto *rng = QRandomGenerator::global();

    for (int i = 0; i != COUNT; ++i) {
        const QByteArray idx = QByteArray::number(i);
        m_data[3 * i + 0] = {"x_" + idx, 0.0};
        m_data[3 * i + 1] = {"y_" + idx, 0.0};
        m_data[3 * i + 2] = {"r_" + idx, 0.0};

        m_speed[i] = rng->generateDouble() + 0.1;
    }

    m_data.append({QByteArray("count"), COUNT});
    m_data.append({QByteArray("fps"), 0});
    m_data.append({QByteArray("title"), m_propertyMap->metaObject()->className()});

    // Initialize with backend-specific init
    switch (m_type) {
    case MapType::Static: initStatic(*m_staticMap, m_data); break;
    case MapType::Qml:    initQml(*m_qmlMap, m_data);       break;
    case MapType::Quick:  initQuick(*m_quickMap, m_data);    break;
    }

    rootContext()->setContextProperty("ngi", m_propertyMap);

    // Pre-resolve property indices for the QuickPropertyMap fast path
    if (m_quickMap) {
        m_cachedIndices.resize(3 * COUNT);
        for (int i = 0; i != COUNT; ++i) {
            m_cachedIndices[3 * i + 0] = m_quickMap->indexOf(m_data[3 * i + 0].first);
            m_cachedIndices[3 * i + 1] = m_quickMap->indexOf(m_data[3 * i + 1].first);
            m_cachedIndices[3 * i + 2] = m_quickMap->indexOf(m_data[3 * i + 2].first);
        }
        m_fpsIndex = m_quickMap->indexOf("fps");
    }

    test1();
    test2();
    test3();
}

void Player::mapInsert(const QByteArray &key, const QVariant &value)
{
    switch (m_type) {
    case MapType::Static: m_staticMap->insert(key, value); break;
    case MapType::Qml:    m_qmlMap->insert(key, value);    break;
    case MapType::Quick:  m_quickMap->insert(key, value);   break;
    }
}

void Player::mapInsertByIndex(int cachedIndex, const QByteArray &key, const QVariant &value)
{
    switch (m_type) {
    case MapType::Static: m_staticMap->insert(key, value);        break;
    case MapType::Qml:    m_qmlMap->insert(key, value);           break;
    case MapType::Quick:  m_quickMap->insert(cachedIndex, value);  break; // O(1) — no hash lookup
    }
}

void Player::test1()
{
    connect(m_timer, &QTimer::timeout, this, &Player::advance);
    m_timer->start(16);
}

void Player::test2()
{
    // valueChanged signal is available on QmlPropertyMap and QuickPropertyMap
    if (m_qmlMap) {
        connect(m_qmlMap, &QmlPropertyMap::valueChanged, [](const QString &name, const QVariant &value) {
            qDebug() << name << value;
        });
    } else if (m_quickMap) {
        connect(m_quickMap, &QuickPropertyMapBase::valueChanged, [](const QByteArray &name, const QVariant &value) {
            qDebug() << name << value;
        });
    }
    // StaticPropertyMap doesn't have a generic valueChanged signal
}

void Player::test3()
{
    qInfo("feeding %s:", m_propertyMap->metaObject()->className());

    for (int size : QList<int>{1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000}) {
        PairData data(size);
        for (int i = 0; i != size; ++i)
            data[i] = {"x_" + QByteArray::number(i), 0.0};

        QElapsedTimer t;
        t.start();

        switch (m_type) {
        case MapType::Static: {
            StaticPropertyMap p;
            initStatic(p, data);
            break;
        }
        case MapType::Qml: {
            QmlPropertyMap p;
            initQml(p, data);
            break;
        }
        case MapType::Quick: {
            QuickPropertyMap p;
            initQuick(p, data);
            break;
        }
        }

        qInfo("%d %lld", size, t.elapsed());
    }
}

void Player::advance()
{
    QElapsedTimer t;
    t.start();

    ++m_step;

    for (int i = 0; i != COUNT; ++i) {
        const double angle = m_step * m_speed[i];
        const int base = 3 * i;

        mapInsertByIndex(m_cachedIndices.value(base + 0, -1), m_data[base + 0].first, posX(i, angle));
        mapInsertByIndex(m_cachedIndices.value(base + 1, -1), m_data[base + 1].first, posY(i, angle));
        mapInsertByIndex(m_cachedIndices.value(base + 2, -1), m_data[base + 2].first, angle + 90);
    }

    if (m_step % 10 == 0)
        mapInsertByIndex(m_fpsIndex, "fps", int(1000.0 / (t.nsecsElapsed() / 1000000.0)));
}
