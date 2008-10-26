/*
 * This file was generated by dbusxml2cpp version 0.6
 * Command line was: dbusxml2cpp -v -m -i kglobalshortcutinfo_p.h -p kdedglobalaccel_interface org.kde.KdedGlobalAccel.xml org.kde.KdedGlobalAccel
 *
 * dbusxml2cpp is Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef KDEDGLOBALACCEL_INTERFACE_H_1224790700
#define KDEDGLOBALACCEL_INTERFACE_H_1224790700

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>
#include "kglobalshortcutinfo_p.h"

/*
 * Proxy class for interface org.kde.KdedGlobalAccel
 */
class OrgKdeKdedGlobalAccelInterface: public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    { return "org.kde.KdedGlobalAccel"; }

public:
    OrgKdeKdedGlobalAccelInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);

    ~OrgKdeKdedGlobalAccelInterface();

public Q_SLOTS: // METHODS
    inline QDBusReply<QStringList> action(int key)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(key);
        return callWithArgumentList(QDBus::Block, QLatin1String("action"), argumentList);
    }

    inline QDBusReply<void> activateGlobalShortcutContext(const QString &component, const QString &context)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(component) << qVariantFromValue(context);
        return callWithArgumentList(QDBus::Block, QLatin1String("activateGlobalShortcutContext"), argumentList);
    }

    inline QDBusReply<QList<QStringList> > allActionsForComponent(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("allActionsForComponent"), argumentList);
    }

    inline QDBusReply<QList<QDBusObjectPath> > allComponents()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("allComponents"), argumentList);
    }

    inline QDBusReply<QList<QStringList> > allMainComponents()
    {
        QList<QVariant> argumentList;
        return callWithArgumentList(QDBus::Block, QLatin1String("allMainComponents"), argumentList);
    }

    inline QDBusReply<QList<int> > defaultShortcut(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("defaultShortcut"), argumentList);
    }

    inline QDBusReply<void> doRegister(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("doRegister"), argumentList);
    }

    inline QDBusReply<bool> isGlobalShortcutAvailable(int key, const QString &component)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(key) << qVariantFromValue(component);
        return callWithArgumentList(QDBus::Block, QLatin1String("isGlobalShortcutAvailable"), argumentList);
    }

    inline QDBusReply<void> setForeignShortcut(const QStringList &actionId, const QList<int> &keys)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId) << qVariantFromValue(keys);
        return callWithArgumentList(QDBus::Block, QLatin1String("setForeignShortcut"), argumentList);
    }

    inline QDBusReply<void> setInactive(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("setInactive"), argumentList);
    }

    inline QDBusReply<QList<int> > setShortcut(const QStringList &actionId, const QList<int> &keys, uint flags)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId) << qVariantFromValue(keys) << qVariantFromValue(flags);
        return callWithArgumentList(QDBus::Block, QLatin1String("setShortcut"), argumentList);
    }

    inline QDBusReply<QList<int> > shortcut(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("shortcut"), argumentList);
    }

    inline QDBusReply<void> unRegister(const QStringList &actionId)
    {
        QList<QVariant> argumentList;
        argumentList << qVariantFromValue(actionId);
        return callWithArgumentList(QDBus::Block, QLatin1String("unRegister"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void invokeAction(const QStringList &actionId, qlonglong timestamp);
    void yourShortcutGotChanged(const QStringList &actionId, const QList<int> &newKeys);
};

namespace org {
  namespace kde {
    typedef ::OrgKdeKdedGlobalAccelInterface KdedGlobalAccel;
  }
}
#endif
