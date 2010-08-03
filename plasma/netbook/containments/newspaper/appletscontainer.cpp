/////////////////////////////////////////////////////////////////////////
// appletscontainer.cpp                                                //
//                                                                     //
// Copyright 2007 by Alex Merry <alex.merry@kdemail.net>               //
// Copyright 2008 by Alexis Ménard <darktears31@gmail.com>            //
// Copyright 2008 by Aaron Seigo <aseigo@kde.org>                      //
// Copyright 2009 by Marco Martin <notmart@gmail.com>                  //
// Copyright 2010 by Igor Oliveira <igor.oliveira@>openbossa.org>      //
//                                                                     //
// This library is free software; you can redistribute it and/or       //
// modify it under the terms of the GNU Lesser General Public          //
// License as published by the Free Software Foundation; either        //
// version 2.1 of the License, or (at your option) any later version.  //
//                                                                     //
// This library is distributed in the hope that it will be useful,     //
// but WITHOUT ANY WARRANTY; without even the implied warranty of      //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   //
// Lesser General Public License for more details.                     //
//                                                                     //
// You should have received a copy of the GNU Lesser General Public    //
// License along with this library; if not, write to the Free Software //
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA       //
// 02110-1301  USA                                                     //
/////////////////////////////////////////////////////////////////////////

#include "appletscontainer.h"
#include "applettitlebar.h"
#include "appletsview.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPropertyAnimation>
#include <QWidget>
#include <QTimer>

#include <KGlobalSettings>
#include <KIconLoader>

#include <Plasma/Applet>
#include <Plasma/ScrollWidget>
#include <Plasma/Containment>

using namespace Plasma;

AppletsContainer::AppletsContainer(AppletsView *parent)
 : QGraphicsWidget(parent),
   m_scrollWidget(parent),
   m_orientation(Qt::Vertical),
   m_viewportSize(size()),
   m_containment(0),
   m_automaticAppletLayout(true),
   m_expandAll(false)
{
    setFlag(QGraphicsItem::ItemHasNoContents);
    m_mainLayout = new QGraphicsLinearLayout(this);
    m_mainLayout->setContentsMargins(0,0,0,0);

    connect(m_scrollWidget, SIGNAL(viewportGeometryChanged(const QRectF &)),
            this, SLOT(viewportGeometryChanged(const QRectF &)));

    m_appletActivationTimer = new QTimer(this);
    m_appletActivationTimer->setSingleShot(true);
    connect(m_appletActivationTimer, SIGNAL(timeout()),
            this, SLOT(delayedAppletActivation()));
}

AppletsContainer::~AppletsContainer()
{
}

void AppletsContainer::setAutomaticAppletLayout(const bool automatic)
{
    m_automaticAppletLayout = automatic;
}

bool AppletsContainer::automaticAppletLayout() const
{
    return m_automaticAppletLayout;
}

void AppletsContainer::syncColumnSizes()
{
    const int margin = 4 + (m_mainLayout->count() - 1) * m_mainLayout->spacing();

    QSizeF viewportSize = m_scrollWidget->viewportGeometry().size();

    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (m_orientation == Qt::Vertical) {
            lay->setMaximumWidth(viewportSize.width()/m_mainLayout->count()-margin);
            lay->setMinimumWidth(viewportSize.width()/m_mainLayout->count()-margin);
            lay->setMaximumHeight(-1);
            lay->setMinimumHeight(-1);
        } else {
            lay->setMaximumHeight(viewportSize.height()/m_mainLayout->count()-margin);
            lay->setMinimumHeight(viewportSize.height()/m_mainLayout->count()-margin);
            lay->setMaximumWidth(-1);
            lay->setMinimumWidth(-1);
        }
    }
}

void AppletsContainer::updateSize()
{
    QSizeF hint = effectiveSizeHint(Qt::PreferredSize);

    //FIXME: it appears to work only with hardcoded values
    if (m_orientation == Qt::Horizontal) {
        resize(qMax((int)hint.width(), 300), qMin(size().height(), m_scrollWidget->viewportGeometry().height()));
    } else {
        resize(qMin(size().width(), m_scrollWidget->viewportGeometry().width()), qMax((int)hint.height(), 300));
    }
}

void AppletsContainer::setExpandAll(const bool expand)
{
    if (m_expandAll == expand) {
        return;
    }

    m_expandAll = expand;

    if (!m_containment) {
        return;
    }

    if (expand && m_orientation != Qt::Horizontal) {
        foreach (Plasma::Applet *applet, m_containment->applets()) {
            if (applet->effectiveSizeHint(Qt::MinimumSize).height() > KIconLoader::SizeSmall) {
                applet->setPreferredHeight(-1);
            }
        }

    } else {
        foreach (Plasma::Applet *applet, m_containment->applets()) {
            if (m_orientation == Qt::Vertical) {
                if (applet == m_currentApplet.data()) {
                    applet->setPreferredHeight(optimalAppletSize(applet, true).height());
                } else {
                    applet->setPreferredHeight(optimalAppletSize(applet, false).height());
                }
            } else {
                applet->setPreferredSize(-1, -1);
                applet->setPreferredWidth((m_scrollWidget->viewportGeometry().size().width()/2)-8);
            }
        }
    }

    if (m_orientation == Qt::Horizontal || (!m_expandAll && !m_currentApplet)) {
        m_scrollWidget->setSnapSize(m_scrollWidget->viewportGeometry().size()/2);
    } else {
        m_scrollWidget->setSnapSize(QSizeF());
    }

    updateSize();
}

bool AppletsContainer::expandAll() const
{
    return m_expandAll;
}

Qt::Orientation AppletsContainer::orientation() const
{
    return m_orientation;
}

Plasma::Containment *AppletsContainer::containment() const
{
    return m_containment;
}

QGraphicsLinearLayout *AppletsContainer::addColumn()
{
    QGraphicsLinearLayout *lay = new QGraphicsLinearLayout(m_orientation);
    lay->setContentsMargins(0,0,0,0);
    m_mainLayout->addItem(lay);

    QGraphicsWidget *spacer = new QGraphicsWidget(this);
    spacer->setPreferredSize(0, 0);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    lay->addItem(spacer);

    syncColumnSizes();

    return lay;
}

void AppletsContainer::cleanupColumns()
{
    //clean up all empty columns
    for (int i = 0; i < count(); ++i) {
        QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        if (!lay) {
            continue;
        }

        if (lay->count() == 1) {
            removeColumn(i);
        }
    }
}

void AppletsContainer::removeColumn(int column)
{
    QGraphicsLinearLayout *lay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(column));

    if (!lay) {
        return;
    }

    m_mainLayout->removeAt(column);

    for (int i = 0; i < lay->count(); ++i) {
        QGraphicsLayoutItem *item = lay->itemAt(i);
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(item);
        Plasma::Applet *applet = qobject_cast<Plasma::Applet *>(widget);

        //find a new home for the applet
        if (applet) {
            layoutApplet(applet, applet->pos());
        //delete spacers
        } else if (widget) {
            widget->deleteLater();
        }
    }

    syncColumnSizes();

    delete lay;
}

void AppletsContainer::layoutApplet(Plasma::Applet* applet, const QPointF &pos)
{
    if (!m_automaticAppletLayout) {
        return;
    }

    QGraphicsLinearLayout *lay = 0;

    for (int i = 0; i < m_mainLayout->count(); ++i) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(i));

        //normally should never happen
        if (!candidateLay) {
            continue;
        }

        if (m_orientation == Qt::Horizontal) {
            if (pos.y() < candidateLay->geometry().bottom()) {
                lay = candidateLay;
                break;
            }
        //vertical
        } else {
            if (pos.x() < candidateLay->geometry().right()) {
                lay = candidateLay;
                break;
            }
        }
    }

    //couldn't decide: is the last column empty?
    if (!lay) {
        QGraphicsLinearLayout *candidateLay = dynamic_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(m_mainLayout->count()-1));

        if (candidateLay && candidateLay->count() == 1) {
            lay = candidateLay;
        }
    }

    //give up, make a new column
    if (!lay) {
        lay = addColumn();
    }

    int insertIndex = -1;

    QPointF localPos = mapToItem(this, pos);

    //if localPos is (-1,-1) insert at the end of the Newspaper
    if (localPos != QPoint(-1, -1)) {
        for (int i = 0; i < lay->count(); ++i) {
            QGraphicsLayoutItem *li = lay->itemAt(i);

            QRectF siblingGeometry = li->geometry();
            if (m_orientation == Qt::Horizontal) {
                qreal middle = (siblingGeometry.left() + siblingGeometry.right()) / 2.0;
                if (localPos.x() < middle) {
                    insertIndex = i;
                    break;
                } else if (localPos.x() <= siblingGeometry.right()) {
                    insertIndex = i + 1;
                    break;
                }
            } else { //Vertical
                qreal middle = (siblingGeometry.top() + siblingGeometry.bottom()) / 2.0;

                if (localPos.y() < middle) {
                    insertIndex = i;
                    break;
                } else if (localPos.y() <= siblingGeometry.bottom()) {
                    insertIndex = i + 1;
                    break;
                }
            }
        }
    }


    if (insertIndex == -1) {
        lay->insertItem(lay->count()-1, applet);
    } else {
        lay->insertItem(qMin(insertIndex, lay->count()-1), applet);
    }

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SIGNAL(appletSizeHintChanged()));
    updateSize();
    createAppletTitle(applet);
    syncColumnSizes();
}

void AppletsContainer::addApplet(Plasma::Applet* applet, const int row, const int column)
{
    QGraphicsLinearLayout *lay;

    //give up, make a new column
    if (column < 0 || column >= m_mainLayout->count()) {
        lay = addColumn();
    } else {
        lay = static_cast<QGraphicsLinearLayout *>(m_mainLayout->itemAt(column));
    }

    if (row <= 0) {
        lay->insertItem(lay->count()-1, applet);
    } else {
        lay->insertItem(qMin(row, lay->count()-1), applet);
    }

    connect(applet, SIGNAL(sizeHintChanged(Qt::SizeHint)), this, SIGNAL(appletSizeHintChanged()));
    updateSize();
    createAppletTitle(applet);
    syncColumnSizes();
}

void AppletsContainer::createAppletTitle(Plasma::Applet *applet)
{
    QList<AppletTitleBar *> titles = applet->findChildren<AppletTitleBar *>("TitleBar");

    if (!titles.isEmpty()) {
        return;
    }

    AppletTitleBar *appletTitleBar = new AppletTitleBar(applet);
    appletTitleBar->setParent(applet);
    appletTitleBar->show();

    if (!m_containment) {
        m_containment = applet->containment();
    }
    if (m_orientation == Qt::Horizontal) {
         applet->setPreferredSize(-1, -1);
         applet->setPreferredWidth((m_scrollWidget->viewportGeometry().size().width()/2)-8);
    } else if (m_expandAll) {
        if (applet->effectiveSizeHint(Qt::MinimumSize).height() > KIconLoader::SizeSmall) {
            applet->setPreferredHeight(-1);
        }
    } else {
        applet->setPreferredHeight(optimalAppletSize(applet, false).height());
    }
}

void AppletsContainer::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    m_mainLayout->setOrientation(orientation==Qt::Vertical?Qt::Horizontal:Qt::Vertical);
}

int AppletsContainer::count() const
{
    return m_mainLayout->count();
}

QGraphicsLayoutItem *AppletsContainer::itemAt(int i)
{
    return m_mainLayout->itemAt(i);
}

QSizeF AppletsContainer::optimalAppletSize(Plasma::Applet *applet, const bool maximized) const
{
    QSizeF normalAppletSize = QSizeF(applet->effectiveSizeHint(Qt::MinimumSize)).expandedTo(m_viewportSize/2) - QSizeF(8, 8);

    //FIXME: it was necessary to add hardcoded fixed qsizes to make things work, why?
    if (maximized) {
        //FIXME: this change of fixed preferred height could cause a relayout, unfortunately there is no other way
        int preferred = applet->preferredHeight();
        applet->setPreferredHeight(-1);
        QSizeF size = QSizeF(applet->effectiveSizeHint(Qt::PreferredSize) + QSizeF(0, 32)).boundedTo(m_viewportSize - QSizeF(12, 12)).expandedTo(normalAppletSize);
        applet->setPreferredHeight(preferred);
        return size;
    } else {
        return normalAppletSize;
    }
}

QSizeF AppletsContainer::viewportSize() const
{
    return m_viewportSize;
}

void AppletsContainer::viewportGeometryChanged(const QRectF &geometry)
{
    m_viewportSize = geometry.size();

    if (!m_containment || (m_expandAll && m_orientation != Qt::Horizontal)) {
        syncColumnSizes();
        return;
    }
    foreach (Plasma::Applet *applet, m_containment->applets()) {
        if (m_orientation == Qt::Vertical) {
            if (applet == m_currentApplet.data()) {
                applet->setPreferredHeight(optimalAppletSize(applet, true).height());
            } else {
                applet->setPreferredHeight(optimalAppletSize(applet, false).height());
            }
        } else {
            applet->setPreferredSize(-1, -1);
            applet->setPreferredWidth((m_scrollWidget->viewportGeometry().size().width()/2)-8);
        }
    }

    if (m_orientation == Qt::Horizontal || (!m_expandAll && !m_currentApplet)) {
        m_scrollWidget->setSnapSize(geometry.size()/2);
    } else {
        m_scrollWidget->setSnapSize(QSizeF());
    }
    syncColumnSizes();
}

void AppletsContainer::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void AppletsContainer::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    if (m_orientation == Qt::Horizontal) {
        return;
    }

    if (m_currentApplet.data()) {
        m_currentApplet.data()->setPreferredHeight(optimalAppletSize(m_currentApplet.data(), false).height());
    }

    if (!m_expandAll) {
        m_scrollWidget->setSnapSize(m_scrollWidget->viewportGeometry().size()/2);
    } else {
        m_scrollWidget->setSnapSize(QSizeF());
    }

    m_pendingCurrentApplet.clear();
    m_currentApplet.clear();

    QGraphicsWidget::mouseReleaseEvent(event);
}



void AppletsContainer::delayedAppletActivation()
{
    if (!m_pendingCurrentApplet) {
        m_currentApplet.clear();
        return;
    }

    m_currentApplet = m_pendingCurrentApplet.data();

    if (m_orientation == Qt::Horizontal || (!m_expandAll && !m_currentApplet)) {
        m_scrollWidget->setSnapSize(m_scrollWidget->viewportGeometry().size()/2);
    } else {
        m_scrollWidget->setSnapSize(QSizeF());
    }
    emit appletActivated(m_currentApplet.data());
}

#include "appletscontainer.moc"

