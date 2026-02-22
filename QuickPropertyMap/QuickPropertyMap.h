#pragma once

#include <QObject>
#include <QList>
#include <QVariant>
#include <QHash>

class QuickPropertyMapBase : public QObject
{
    Q_OBJECT

public:
    explicit QuickPropertyMapBase(QObject *parent = nullptr) : QObject(parent) {}
    ~QuickPropertyMapBase() override = default;

signals:
    void valueChanged(const QByteArray &name, const QVariant &value);
};

class QuickPropertyMap : public QuickPropertyMapBase
{
    // Meta-object system functions
public:
    const QMetaObject *metaObject() const override;
    void *qt_metacast(const char *) override;
    int qt_metacall(QMetaObject::Call, int, void **) override;

public:
    explicit QuickPropertyMap(QObject *parent = nullptr);
    ~QuickPropertyMap() override;

    /**
     * Appends a property to the property list.
     * Has no effect after build() is called.
     *
     * Two-argument version uses value.metaType().id() as a property type.
     *
     * Call build() to create a metaobject and finish QuickPropertyMap creation.
     */
    void addProperty(const QByteArray &name, const QVariant &value, int type);
    void addProperty(const QByteArray &name, const QVariant &value) { addProperty(name, value, value.metaType().id()); }

    /**
     * @brief Builds a QMetaObject from the property list accumulated by addProperty() calls.
     */
    void build();

    /**
     * @brief Updates property at 'index' with 'value'.
     */
    void insert(int index, const QVariant &value);
    void insert(const QByteArray &name, const QVariant &value) { insert(indexOf(name), value); }

    int count() const { return m_propertyIndex.count(); }
    int indexOf(const QByteArray &name) const { return m_propertyIndex.value(name, -1); }
    bool contains(const QByteArray &name) const { return m_propertyIndex.contains(name); }
    QByteArrayList keys() const { return m_propertyIndex.keys(); }

    const QByteArray &name(int index) const { return m_propertyList[index].name; }
    const QVariant &value(int index) const { return m_propertyList[index].value; }
    int type(int index) const { return m_propertyList[index].typeId; }

    QVariant value(const QByteArray &name) const { int i = indexOf(name); return (i != -1) ? value(i) : QVariant(); }

private:
    int my_metacall(QMetaObject::Call call, int id, void **argv);
    void buildMetaObject();

    struct DynamicProperty {
        QByteArray name;
        QVariant value;
        int typeId;
    };

    QMetaObject *m_metaObject = nullptr;
    bool m_finalized = false;

    QHash<QByteArray, int> m_propertyIndex;
    QList<DynamicProperty> m_propertyList;
};
