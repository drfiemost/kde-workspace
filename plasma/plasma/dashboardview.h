/*
 *   Copyright 2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2007 André Duffeck <duffeck@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef DASHBOARDVIEW_H
#define DASHBOARDVIEW_H

#include "plasma/plasma.h"
#include "plasma/view.h"

namespace Plasma
{
    class AppletBrowserWidget;
}

class DashboardView : public Plasma::View
{
    Q_OBJECT

public:
    DashboardView(int screen, QWidget *parent);
    ~DashboardView();

protected:
    void drawBackground(QPainter * painter, const QRectF & rect);
    void keyPressEvent(QKeyEvent *event);

public slots:
    void toggleVisibility();

protected slots:
    void showAppletBrowser();
    void appletBrowserDestroyed();
    void hideView();
    void suppressShowTimeout();

private:
    Plasma::AppletBrowserWidget *m_appletBrowserWidget;
    bool m_suppressShow;
    bool m_zoomIn;
    bool m_zoomOut;
};

#endif // multiple inclusion guard
