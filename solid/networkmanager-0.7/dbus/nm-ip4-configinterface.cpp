/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -N -m -p nm-ip4-configinterface /space/kde/sources/trunk/KDE/kdebase/workspace/solid/networkmanager-0.7/dbus/introspection/nm-ip4-config.xml
 *
 * qdbusxml2cpp is Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#include "nm-ip4-configinterface.h"

/*
 * Implementation of interface class OrgFreedesktopNetworkManagerIP4ConfigInterface
 */

OrgFreedesktopNetworkManagerIP4ConfigInterface::OrgFreedesktopNetworkManagerIP4ConfigInterface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent)
{
}

OrgFreedesktopNetworkManagerIP4ConfigInterface::~OrgFreedesktopNetworkManagerIP4ConfigInterface()
{
}


#include "nm-ip4-configinterface.moc"