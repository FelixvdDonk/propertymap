#pragma once

#include <QtQml/qtqmlglobal.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

class QmlPropertyMapPrivate;

/**
 * @brief A reimplementation of QQmlPropertyMap using Qt 6's QQmlOpenMetaObject.
 *
 * This provides a dynamic property map that is directly usable from QML,
 * acting as a faster alternative to Qt's built-in QQmlPropertyMap for
 * high-frequency property updates.
 */
class QmlPropertyMap : public QObject
{
    Q_OBJECT

public:
    using FastData = QHash<QByteArray, QVariant>;
    using PairData = QList<QPair<QByteArray, QVariant>>;

    explicit QmlPropertyMap(QObject *parent = nullptr);
    ~QmlPropertyMap() override;

    QVariant value(const QString &key) const;
    void clear(const QString &key);

    void insert(const QString &key, const QVariant &value);
    void insert(const QVariantMap &data);
    void insert(const FastData &data);
    void insert(const PairData &data);

    void setCached(bool cached);

    Q_INVOKABLE const QStringList &keys() const;

    int count() const;
    int size() const;
    bool isEmpty() const;
    bool contains(const QString &key) const;

    QVariant &operator[](const QString &key);
    QVariant operator[](const QString &key) const;

Q_SIGNALS:
    void valueChanged(const QString &key, const QVariant &value);

protected:
    virtual QVariant updateValue(const QString &key, const QVariant &input);

    template<class DerivedType>
    QmlPropertyMap(DerivedType *derived, QObject *parentObj)
        : QObject(*allocatePrivate(), parentObj)
    {
        Q_UNUSED(derived)
        init(&DerivedType::staticMetaObject);
    }

private:
    void init(const QMetaObject *staticMetaObject);
    static QObjectPrivate *allocatePrivate();

    Q_DECLARE_PRIVATE(QmlPropertyMap)
    Q_DISABLE_COPY(QmlPropertyMap)
};
