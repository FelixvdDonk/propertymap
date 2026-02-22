#include "QmlPropertyMap.h"

#include <private/qqmlopenmetaobject_p.h>
#include <private/qmetaobjectbuilder_p.h>

#include <QDebug>

class QmlPropertyMapMetaObject : public QQmlOpenMetaObject
{
public:
    QmlPropertyMapMetaObject(QmlPropertyMap *obj, QmlPropertyMapPrivate *objPriv, const QMetaObject *staticMetaObject);

protected:
    QVariant propertyWriteValue(int, const QVariant &) override;
    void propertyWritten(int index) override;
    void propertyCreated(int, QMetaPropertyBuilder &) override;
    int createProperty(const char *, const char *) override;

    const QString &propertyName(int index);

private:
    QmlPropertyMap *map;
    QmlPropertyMapPrivate *priv;
};

class QmlPropertyMapPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QmlPropertyMap)
public:
    QmlPropertyMapMetaObject *mo = nullptr;
    QSet<QString> lookup;
    QStringList keys;

    QVariant updateValue(const QString &key, const QVariant &input);
    void emitChanged(const QString &key, const QVariant &value);
    bool validKeyName(const QString &name);

    const QString &propertyName(int index) const;
};

bool QmlPropertyMapPrivate::validKeyName(const QString &name)
{
    return name != QLatin1String("keys")
        && name != QLatin1String("valueChanged")
        && name != QLatin1String("QObject")
        && name != QLatin1String("destroyed")
        && name != QLatin1String("deleteLater");
}

QVariant QmlPropertyMapPrivate::updateValue(const QString &key, const QVariant &input)
{
    Q_Q(QmlPropertyMap);
    return q->updateValue(key, input);
}

void QmlPropertyMapPrivate::emitChanged(const QString &key, const QVariant &value)
{
    Q_Q(QmlPropertyMap);
    emit q->valueChanged(key, value);
}

const QString &QmlPropertyMapPrivate::propertyName(int index) const
{
    Q_ASSERT(index < keys.size());
    return keys[index];
}

QmlPropertyMapMetaObject::QmlPropertyMapMetaObject(QmlPropertyMap *obj, QmlPropertyMapPrivate *objPriv, const QMetaObject *staticMetaObject)
    : QQmlOpenMetaObject(obj, staticMetaObject)
{
    map = obj;
    priv = objPriv;
}

QVariant QmlPropertyMapMetaObject::propertyWriteValue(int index, const QVariant &input)
{
    return priv->updateValue(priv->propertyName(index), input);
}

void QmlPropertyMapMetaObject::propertyWritten(int index)
{
    priv->emitChanged(priv->propertyName(index), value(index));
}

void QmlPropertyMapMetaObject::propertyCreated(int, QMetaPropertyBuilder &b)
{
    priv->keys.append(QString::fromUtf8(b.name()));
    priv->lookup.insert(priv->keys.last());
}

int QmlPropertyMapMetaObject::createProperty(const char *name, const char *value)
{
    if (!priv->validKeyName(QString::fromUtf8(name)))
        return -1;
    return QQmlOpenMetaObject::createProperty(name, value);
}

QmlPropertyMap::QmlPropertyMap(QObject *parent)
    : QObject(*allocatePrivate(), parent)
{
    init(metaObject());
}

QmlPropertyMap::~QmlPropertyMap()
{}

void QmlPropertyMap::clear(const QString &key)
{
    Q_D(QmlPropertyMap);
    d->mo->setValue(key.toUtf8(), QVariant());
}

QVariant QmlPropertyMap::value(const QString &key) const
{
    Q_D(const QmlPropertyMap);
    return d->mo->value(key.toUtf8());
}

void QmlPropertyMap::insert(const QString &key, const QVariant &value)
{
    Q_D(QmlPropertyMap);

    if (d->validKeyName(key)) {
        d->mo->setValue(key.toUtf8(), value);
    } else {
        qWarning() << "Creating property with name"
                    << key
                    << "is not permitted, conflicts with internal symbols.";
    }
}

void QmlPropertyMap::insert(const QVariantMap &data)
{
    Q_D(QmlPropertyMap);

    QHash<QByteArray, QVariant> tmp;

    for (auto i = data.cbegin(), e = data.cend(); i != e; ++i) {
        if (d->validKeyName(i.key())) {
            tmp.insert(i.key().toUtf8(), i.value());
        } else {
            qWarning() << "Creating property with name"
                        << i.key()
                        << "is not permitted, conflicts with internal symbols.";
        }
    }

    d->mo->setValues(tmp);
}

void QmlPropertyMap::insert(const FastData &data)
{
    Q_D(QmlPropertyMap);
    d->mo->setValues(data);
}

void QmlPropertyMap::insert(const PairData &data)
{
    Q_D(QmlPropertyMap);
    // Convert PairData to QHash for the Qt 6 API
    QHash<QByteArray, QVariant> hash;
    hash.reserve(data.size());
    for (const auto &pair : data)
        hash.insert(pair.first, pair.second);
    d->mo->setValues(hash);
}

void QmlPropertyMap::setCached(bool cached)
{
    Q_D(QmlPropertyMap);
    d->mo->setCached(cached);
}

const QStringList &QmlPropertyMap::keys() const
{
    Q_D(const QmlPropertyMap);
    return d->keys;
}

int QmlPropertyMap::count() const
{
    Q_D(const QmlPropertyMap);
    return d->keys.count();
}

int QmlPropertyMap::size() const
{
    Q_D(const QmlPropertyMap);
    return d->keys.size();
}

bool QmlPropertyMap::isEmpty() const
{
    Q_D(const QmlPropertyMap);
    return d->keys.isEmpty();
}

bool QmlPropertyMap::contains(const QString &key) const
{
    Q_D(const QmlPropertyMap);
    return d->lookup.contains(key);
}

QVariant &QmlPropertyMap::operator[](const QString &key)
{
    Q_D(QmlPropertyMap);
    QByteArray utf8key = key.toUtf8();
    if (!d->lookup.contains(key))
        insert(key, QVariant()); // force creation
    return d->mo->valueRef(utf8key);
}

QVariant QmlPropertyMap::operator[](const QString &key) const
{
    return value(key);
}

QVariant QmlPropertyMap::updateValue(const QString &key, const QVariant &input)
{
    Q_UNUSED(key)
    return input;
}

void QmlPropertyMap::init(const QMetaObject *staticMetaObject)
{
    Q_D(QmlPropertyMap);
    d->mo = new QmlPropertyMapMetaObject(this, d, staticMetaObject);
}

QObjectPrivate *QmlPropertyMap::allocatePrivate()
{
    return new QmlPropertyMapPrivate;
}
