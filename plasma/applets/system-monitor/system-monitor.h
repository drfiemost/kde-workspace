/*
 *   Copyright (C) 2007 Petri Damsten <damu@iki.fi>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SYSTEM_MONITOR_HEADER
#define SYSTEM_MONITOR_HEADER

#include <Plasma/PopupApplet>
#include <Plasma/DataEngine>

namespace SM {
    class Applet;
}
class MonitorButton;

class QGraphicsLinearLayout;

class SystemMonitor : public Plasma::PopupApplet
{
    Q_OBJECT
    public:
        SystemMonitor(QObject *parent, const QVariantList &args);
        virtual ~SystemMonitor();

        void init();
        virtual QList<QAction*> contextualActions();
        virtual QGraphicsWidget *graphicsWidget();
        virtual void constraintsEvent(Plasma::Constraints constraints);

    public slots:
        void checkGeometry();

    protected slots:
        void toggled(bool toggled);
        void appletRemoved(QObject *object);

    protected:
        void addApplet(const QString &name);
        void removeApplet(const QString &name);

    private:
        QGraphicsLinearLayout *m_layout;
        QGraphicsLinearLayout *m_buttons;
        QList<SM::Applet*> m_applets;
        QList<MonitorButton*> m_monitorButtons;
        QGraphicsWidget *m_widget;
};

K_EXPORT_PLASMA_APPLET(system-monitor_applet, SystemMonitor)

#endif
