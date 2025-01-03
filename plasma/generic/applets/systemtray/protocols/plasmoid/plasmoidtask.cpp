/***************************************************************************
 *   plasmoidtask.cpp                                                      *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Sebastian Kügler <sebas@kde.org>                   *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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

#include "plasmoidtask.h"
#include <fixx11h.h>

#include <KIcon>
#include <KIconLoader>

#include <plasma/applet.h>
#include <plasma/popupapplet.h>
#include <plasma/plasma.h>

#include <kdebug.h>


namespace SystemTray
{

PlasmoidTask::PlasmoidTask(const QString &appletname, int id, QObject *parent, Plasma::Applet *host)
    : Task(parent),
      m_appletName(appletname),
      m_taskId(appletname),
      m_host(host),
      m_takenByParent(false)
{
    setName(appletname);
    setupApplet(appletname, id);
}


PlasmoidTask::~PlasmoidTask()
{
    emit taskDeleted(m_host, m_taskId);
}


bool PlasmoidTask::isEmbeddable() const
{
    return m_applet && !m_takenByParent;
}

bool PlasmoidTask::isValid() const
{
    return !m_appletName.isEmpty() && m_applet;
}


QString PlasmoidTask::taskId() const
{
    return m_taskId;
}


QIcon PlasmoidTask::icon() const
{
    return m_icon;
}

Plasma::Applet *PlasmoidTask::host() const
{
    return m_host;
}

bool PlasmoidTask::isWidget() const {
    return true;
}

QGraphicsWidget* PlasmoidTask::createWidget(Plasma::Applet *host)
{
    if (host != m_host || !m_applet) {
        return 0;
    }

    Plasma::Applet *applet = m_applet.data();
    m_takenByParent = true;
    applet->setParent(host);
    applet->setParentItem(host);
    KConfigGroup group = applet->config();
    group = group.parent();
    applet->restore(group);
    applet->init();
    applet->updateConstraints(Plasma::StartupCompletedConstraint);
    applet->flushPendingConstraintsEvents();
    applet->updateConstraints(Plasma::AllConstraints);
    applet->flushPendingConstraintsEvents();

    // make sure to record it in the configuration so that if we reload from the config,
    // this applet is remembered
    KConfigGroup dummy;
    applet->save(dummy);

    connect(applet, SIGNAL(newStatus(Plasma::ItemStatus)), this, SLOT(newAppletStatus(Plasma::ItemStatus)));

    newAppletStatus(applet->status());

    connect(applet, SIGNAL(configNeedsSaving()), host, SIGNAL(configNeedsSaving()));
    connect(applet, SIGNAL(releaseVisualFocus()), host, SIGNAL(releaseVisualFocus()));

    return static_cast<QGraphicsWidget*>(applet);
}

void PlasmoidTask::forwardConstraintsEvent(Plasma::Constraints constraints)
{
    Plasma::Applet *applet = m_applet.data();
    if (applet) {
        applet->updateConstraints(constraints);
        applet->flushPendingConstraintsEvents();
    }
}

int PlasmoidTask::id() const
{
    if (m_applet) {
        return m_applet.data()->id();
    }

    return 0;
}

void PlasmoidTask::setupApplet(const QString &plugin, int id)
{
    Plasma::Applet *applet = Plasma::Applet::load(plugin, id);
    m_applet = applet;

    if (!m_applet) {
        kDebug() << "Could not load applet" << plugin;
        return;
    }

    //FIXME: System Information should be system services, but battery and devicenotifier are both there. we would need multiple categories
    if (applet->category() == "System Information" ||
        applet->category() == "Network") {
        setCategory(Hardware);
    } else if (applet->category() == "Online Services") {
        setCategory(Communications);
    }

    setName(applet->name());

    m_icon = KIcon(applet->icon());

    applet->setFlag(QGraphicsItem::ItemIsMovable, false);

    connect(applet, SIGNAL(appletDestroyed(Plasma::Applet*)), this, SLOT(appletDestroyed(Plasma::Applet*)));
    applet->setBackgroundHints(Plasma::Applet::NoBackground);
}

void PlasmoidTask::appletDestroyed(Plasma::Applet *)
{
    emit destroyed(this);
    forget(m_host);
    deleteLater();
}

void PlasmoidTask::newAppletStatus(Plasma::ItemStatus status)
{
    Plasma::Applet *applet = m_applet.data();
    if (!applet) {
        return;
    }

    switch (status) {
    case Plasma::PassiveStatus:
       if (Plasma::PopupApplet *popupApplet = qobject_cast<Plasma::PopupApplet *>(applet)) {
           popupApplet->hidePopup();
       }
       setStatus(Task::Passive);
       break;

    case Plasma::ActiveStatus:
       setStatus(Task::Active);
       break;

    case Plasma::NeedsAttentionStatus:
        setStatus(Task::NeedsAttention);
        break;

    default:
    case Plasma::UnknownStatus:
        setStatus(Task::UnknownStatus);
    }
}

}

#include "plasmoidtask.moc"
