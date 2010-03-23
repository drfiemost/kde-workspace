/***************************************************************************
 *   Copyright (C) 2008 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "CapabilitiesPage.h"

#include "PowerDevilSettings.h"

#include <config-X11.h>
#include <config-workspace.h>
#include <config-powerdevil.h>

#include <solid/control/powermanager.h>
#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/processor.h>
#include <solid/battery.h>

#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>

#include <QX11Info>

#include <QProcess>
#include <KPushButton>
#include <KMessageBox>

#ifdef HAVE_DPMS
#include <X11/Xmd.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
extern "C" {
#include <X11/extensions/dpms.h>
    int __kde_do_not_unload = 1;

#ifndef HAVE_DPMSCAPABLE_PROTO
    Bool DPMSCapable(Display *);
#endif

#ifndef HAVE_DPMSINFO_PROTO
    Status DPMSInfo(Display *, CARD16 *, BOOL *);
#endif
}
#endif

CapabilitiesPage::CapabilitiesPage(QWidget *parent)
        : QWidget(parent)
{
    setupUi(this);

    fillUi();
}

CapabilitiesPage::~CapabilitiesPage()
{
}

void CapabilitiesPage::fillUi()
{
}

void CapabilitiesPage::load()
{
    fillCapabilities();
}

void CapabilitiesPage::fillCapabilities()
{
    int batteryCount = 0;
    int cpuCount = 0;

    foreach(const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::Battery, QString())) {
        Solid::Device dev = device;
        Solid::Battery *b = qobject_cast<Solid::Battery*> (dev.asDeviceInterface(Solid::DeviceInterface::Battery));
        if(b->type() != Solid::Battery::PrimaryBattery) {
            continue;
        }
        batteryCount++;
    }

    foreach(const Solid::Device &device, Solid::Device::listFromType(Solid::DeviceInterface::Processor, QString())) {
        /*Solid::Device d = device;
        Solid::Processor *processor = qobject_cast<Solid::Processor*>(d.asDeviceInterface(Solid::DeviceInterface::Processor));

        if (processor->canChangeFrequency()) {
            freqchange = true;
        }*/
        Q_UNUSED(device)
        ++cpuCount;
    }

    cpuNumber->setText(QString::number(cpuCount));
    batteriesNumber->setText(QString::number(batteryCount));

    QString sMethods;

    Solid::Control::PowerManager::SuspendMethods methods = Solid::Control::PowerManager::supportedSuspendMethods();

    if (methods & Solid::Control::PowerManager::ToDisk) {
        sMethods.append(QString(i18n("Suspend to Disk") + QString(", ")));
    }

    if (methods & Solid::Control::PowerManager::ToRam) {
        sMethods.append(QString(i18n("Suspend to RAM") + QString(", ")));
    }

    if (methods & Solid::Control::PowerManager::Standby) {
        sMethods.append(QString(i18n("Standby") + QString(", ")));
    }

    if (!sMethods.isEmpty()) {
        sMethods.remove(sMethods.length() - 2, 2);
    } else {
        sMethods = i18nc("None", "No methods found");
    }

    supportedMethods->setText(sMethods);

    if (!Solid::Control::PowerManager::supportedSchemes().isEmpty()) {
        isSchemeSupported->setPixmap(KIcon("dialog-ok-apply").pixmap(16, 16));
    } else {
        isSchemeSupported->setPixmap(KIcon("dialog-cancel").pixmap(16, 16));
    }

    QString schemes;

    foreach(const QString &scheme, Solid::Control::PowerManager::supportedSchemes()) {
        schemes.append(scheme + QString(", "));
    }

    if (!schemes.isEmpty()) {
        schemes.remove(schemes.length() - 2, 2);
    } else {
        schemes = i18nc("None", "No methods found");
    }

    supportedSchemes->setText(schemes);

    bool dpms = false;

#ifdef HAVE_DPMS
    Display *dpy = QX11Info::display();

    int dummy;

    if (DPMSQueryExtension(dpy, &dummy, &dummy) && DPMSCapable(dpy)) {
        dpms = true;
    }
#endif

    bool ck;

    if (!QDBusConnection::systemBus().interface()->isServiceRegistered("org.freedesktop.ConsoleKit")) {
        // No way to determine if we are on the current session, simply suppose we are
        ck = false;
    } else {
        QDBusInterface ckiface("org.freedesktop.ConsoleKit", "/org/freedesktop/ConsoleKit/Manager",
                               "org.freedesktop.ConsoleKit.Manager", QDBusConnection::systemBus());

        QDBusReply<QDBusObjectPath> sessionPath = ckiface.call("GetCurrentSession");

        if (!sessionPath.isValid() || sessionPath.value().path().isEmpty()) {
            kDebug() << "The session is not registered with ck";
            ck = false;
        } else {
            QDBusInterface ckSessionInterface("org.freedesktop.ConsoleKit", sessionPath.value().path(),
                                              "org.freedesktop.ConsoleKit.Session", QDBusConnection::systemBus());

            if (!ckSessionInterface.isValid()) {
                ck = false;
            } else {
                ck = true;
            }
        }
    }

    if (dpms) {
        dpmsSupport->setPixmap(KIcon("dialog-ok-apply").pixmap(16, 16));
    } else {
        dpmsSupport->setPixmap(KIcon("dialog-cancel").pixmap(16, 16));
    }

    if (ck) {
        ckSupport->setPixmap(KIcon("dialog-ok-apply").pixmap(16, 16));
    } else {
        ckSupport->setPixmap(KIcon("dialog-cancel").pixmap(16, 16));
    }

    // Determine status!

    foreach(QObject *ent, statusLayout->children()) {
        ent->deleteLater();
    }

    if (!ck) {
        setIssue(true, i18n("ConsoleKit was not found active on your PC, or PowerDevil cannot contact it. "
                            "ConsoleKit lets PowerDevil detect whether the current session is active, which is "
                            "useful if you have more than one user logged into your system at any one time."));

    } else {
        setIssue(false, i18n("No issues found with your configuration."));
    }
}

void CapabilitiesPage::setIssue(bool issue, const QString &text,
                                const QString &button, const QString &buticon, const char *slot,
                                const QString &button2, const QString &buticon2, const char *slot2)
{
    QLabel *pm = new QLabel(this);
    QLabel *tx = new QLabel(this);
    QHBoxLayout *ly = new QHBoxLayout();
    QHBoxLayout *butly = 0;

    pm->setMaximumWidth(30);
    tx->setScaledContents(true);
    tx->setWordWrap(true);

    ly->addWidget(pm);
    ly->addWidget(tx);

    if (!issue) {
        pm->setPixmap(KIcon("dialog-ok-apply").pixmap(16, 16));
        tx->setText(text);
        emit issuesFound(false);
    } else {
        pm->setPixmap(KIcon("dialog-warning").pixmap(16, 16));
        tx->setText(text);

        if (!button.isEmpty()) {
            butly = new QHBoxLayout();
            KPushButton *but = new KPushButton(this);

            but->setText(button);
            but->setIcon(KIcon(buticon));

            ly->removeWidget(tx);

            butly->addStretch();
            butly->addWidget(but);

            QVBoxLayout *vl = new QVBoxLayout();

            vl->addWidget(tx);
            vl->addLayout(butly);

            ly->addLayout(vl);

            connect(but, SIGNAL(clicked()), slot);
        }
        if (!button2.isEmpty()) {
            KPushButton *but = new KPushButton(this);

            but->setText(button2);
            but->setIcon(KIcon(buticon2));

            butly->addWidget(but);

            connect(but, SIGNAL(clicked()), slot2);
        }
        if (!button.isEmpty()) {
            butly->addStretch();
        }

        emit issuesFound(true);
    }

    statusLayout->addLayout(ly);
}

void CapabilitiesPage::enableXSync()
{
    PowerDevilSettings::setPollingSystem(2);

    PowerDevilSettings::self()->writeConfig();

    emit reload();
    emit reloadModule();
}

#include "CapabilitiesPage.moc"
