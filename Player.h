#pragma once

#include <QQuickView>
#include <QList>
#include <QPair>
#include <QVariant>
#include <QString>

class QTimer;

class StaticPropertyMap;
class QmlPropertyMap;
class QuickPropertyMap;

/**
 * @brief Selects which PropertyMap backend to use at runtime.
 */
enum class MapType {
    Static,
    Qml,
    Quick
};

class Player : public QQuickView
{
    Q_OBJECT

public:
    explicit Player(MapType type, QWindow *parent = nullptr);

    void advance();

    void test1();
    void test2();
    void test3();

private:
    /// Type-erased insert that dispatches to the correct backend
    void mapInsert(const QByteArray &key, const QVariant &value);
    /// Fast insert by cached index (QuickPropertyMap only, falls back to key-based)
    void mapInsertByIndex(int cachedIndex, const QByteArray &key, const QVariant &value);

    MapType m_type;
    QObject *m_propertyMap = nullptr;          // owned, one of the three types
    StaticPropertyMap  *m_staticMap  = nullptr; // non-owning alias
    QmlPropertyMap     *m_qmlMap    = nullptr;
    QuickPropertyMap   *m_quickMap  = nullptr;

    QList<double> m_speed;
    QList<QPair<QByteArray, QVariant>> m_data;
    QList<int> m_cachedIndices; // pre-resolved indices for QuickPropertyMap fast path
    int m_fpsIndex = -1;       // cached index for the "fps" property
    QTimer *m_timer;
    int m_step;
};
