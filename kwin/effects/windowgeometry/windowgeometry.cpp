/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

 Copyright (C) 2010 Thomas Lübking <thomas.luebking@web.de>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/

#include "windowgeometry.h"
#include <QStringBuilder>
#include <kwinconfig.h>
#include <kconfiggroup.h>
#include <kwindowsystem.h>
#include <KActionCollection>
#include <kaction.h>
#include <KDE/KLocale>

using namespace KWin;

KWIN_EFFECT(windowgeometry, WindowGeometry)

WindowGeometry::WindowGeometry()
{
    iAmActivated = true;
    iAmActive = false;
    myResizeWindow = 0L;
    myResizeString =   i18nc("Window geometry display, %1 and %2 are the new size,"
                             " %3 and %4 are pixel increments - avoid reformatting or suffixes like 'px'",
                             "Width: %1 (%3)\nHeight: %2 (%4)");
    myCoordString[0] = i18nc("Window geometry display, %1 and %2 are the cartesian x and y coordinates"
                             " - avoid reformatting or suffixes like 'px'",
                             "X: %1\nY: %2");
    myCoordString[1] = i18nc("Window geometry display, %1 and %2 are the cartesian x and y coordinates,"
                             " %3 and %4 are the resp. increments - avoid reformatting or suffixes like 'px'",
                             "X: %1 (%3)\nY: %2 (%4)");
    reconfigure(ReconfigureAll);
    QFont fnt; fnt.setBold(true); fnt.setPointSize(12);
    for (int i = 0; i < 3; ++i) {
        myMeasure[i] = effects->effectFrame(EffectFrameUnstyled, false);
        myMeasure[i]->setFont(fnt);
    }
    myMeasure[0]->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    myMeasure[1]->setAlignment(Qt::AlignCenter);
    myMeasure[2]->setAlignment(Qt::AlignRight | Qt::AlignBottom);

    KActionCollection* actionCollection = new KActionCollection(this);
    KAction* a = static_cast< KAction* >(actionCollection->addAction("WindowGeometry"));
    a->setText(i18n("Toggle window geometry display (effect only)"));
    a->setGlobalShortcut(KShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_F11));
    connect(a, SIGNAL(triggered(bool)), this, SLOT(toggle()));
    connect(effects, SIGNAL(windowStartUserMovedResized(EffectWindow*)), this, SLOT(slotWindowStartUserMovedResized(EffectWindow*)));
    connect(effects, SIGNAL(windowFinishUserMovedResized(EffectWindow*)), this, SLOT(slotWindowFinishUserMovedResized(EffectWindow*)));
    connect(effects, SIGNAL(windowStepUserMovedResized(EffectWindow*,QRect)), this, SLOT(slotWindowStepUserMovedResized(EffectWindow*,QRect)));
}

WindowGeometry::~WindowGeometry()
{
    for (int i = 0; i < 3; ++i)
        delete myMeasure[i];
}

void WindowGeometry::reconfigure(ReconfigureFlags)
{
    KConfigGroup conf = effects->effectConfig("WindowGeometry");
    iHandleMoves = conf.readEntry("Move", true);
    iHandleResizes = conf.readEntry("Resize", true);
}

void WindowGeometry::paintScreen(int mask, QRegion region, ScreenPaintData &data)
{
    effects->paintScreen(mask, region, data);
    if (iAmActivated && iAmActive) {
        for (int i = 0; i < 3; ++i)
            myMeasure[i]->render(infiniteRegion(), 1.0, .66);
    }
}

void WindowGeometry::toggle()
{
    iAmActivated = !iAmActivated;
}

void WindowGeometry::slotWindowStartUserMovedResized(EffectWindow *w)
{
    if (!iAmActivated)
        return;
    if (w->isUserResize() && !iHandleResizes)
        return;
    if (w->isUserMove() && !iHandleMoves)
        return;

    iAmActive = true;
    myResizeWindow = w;
    myOriginalGeometry = w->geometry();
    myCurrentGeometry = w->geometry();
    effects->addRepaint(myCurrentGeometry.adjusted(-20, -20, 20, 20));
}

void WindowGeometry::slotWindowFinishUserMovedResized(EffectWindow *w)
{
    if (iAmActive && w == myResizeWindow) {
        iAmActive = false;
        myResizeWindow = 0L;
        effects->addRepaint(myCurrentGeometry.adjusted(-20, -20, 20, 20));
    }
}

static inline QString number(int n)
{
    if (n >= 0)
        return  '+' + QString::number(n);
    return QString::number(n); // "-" is auto-applied
}


void WindowGeometry::slotWindowStepUserMovedResized(EffectWindow *w, const QRect &geometry)
{
    if (iAmActivated && iAmActive && w == myResizeWindow) {
        myCurrentGeometry = geometry;
        const QRect &r = geometry;
        const QRect &r2 = myOriginalGeometry;

        // sufficient for moves, resizes calculated otherwise
        int dx = r.x() - r2.x();
        int dy = r.y() - r2.y();

        // upper left ----------------------
        if (w->isUserResize())
            myMeasure[0]->setText(myCoordString[1].arg(r.x()).arg(r.y()).arg(number(dx)).arg(number(dy)));
        else
            myMeasure[0]->setText(myCoordString[0].arg(r.x()).arg(r.y()));
        myMeasure[0]->setPosition(geometry.topLeft());

        // center ----------------------
        if (w->isUserResize()) {
            // calc width for center element, otherwise the current dx/dx remains right
            dx = r.width() - r2.width();
            dy = r.height() - r2.height();

            // TODO: i hate this. anyone got a nice idea to invoke the stringbuilder or otherwise avoid
            // dogslow QString::arg() system here?
            myMeasure[1]->setText(myResizeString.arg(r.width()).arg(r.height()).arg(number(dx)).arg(number(dy)));

            // calc width for bottomright element, superfluous otherwise
            dx = r.right() - r2.right();
            dy = r.bottom() - r2.bottom();
        } else
            myMeasure[1]->setText(myCoordString[0].arg(number(dx)).arg(number(dy)));

        myMeasure[1]->setPosition(geometry.center());

        // lower right ----------------------
        if (w->isUserResize())
            myMeasure[2]->setText(myCoordString[1].arg(r.right()).arg(r.bottom()).arg(number(dx)).arg(number(dy)));
        else
            myMeasure[2]->setText(myCoordString[0].arg(r.right()).arg(r.bottom()));
        myMeasure[2]->setPosition(geometry.bottomRight());

        effects->addRepaint(geometry.adjusted(-20, -20, 20, 20));
    }
}

