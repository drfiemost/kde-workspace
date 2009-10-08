/*
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2,
 *   or (at your option) any later version.
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

#include "itemview.h"
#include "itemcontainer.h"

#include <QGraphicsSceneResizeEvent>

#include <Plasma/IconWidget>

ItemView::ItemView(QGraphicsWidget *parent)
    : Plasma::ScrollWidget(parent)
{
    setFocusPolicy(Qt::StrongFocus);
    m_itemContainer = new ItemContainer(this);
    setWidget(m_itemContainer);
    m_itemContainer->installEventFilter(this);

    connect(m_itemContainer, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SIGNAL(itemSelected(Plasma::IconWidget *)));
    connect(m_itemContainer, SIGNAL(itemActivated(Plasma::IconWidget *)), this, SIGNAL(itemActivated(Plasma::IconWidget *)));
    connect(m_itemContainer, SIGNAL(resetRequested()), this, SIGNAL(resetRequested()));
    connect(m_itemContainer, SIGNAL(itemSelected(Plasma::IconWidget *)), this, SLOT(selectItem(Plasma::IconWidget *)));
}

ItemView::~ItemView()
{}

void ItemView::selectItem(Plasma::IconWidget *icon)
{
    QRectF iconRectToMainWidget = icon->mapToItem(m_itemContainer, icon->boundingRect()).boundingRect();

    ensureRectVisible(iconRectToMainWidget);
}

void ItemView::setCurrentItem(Plasma::IconWidget *currentIcon)
{
    m_itemContainer->setCurrentItem(currentIcon);
}

Plasma::IconWidget *ItemView::currentItem() const
{
    return m_itemContainer->currentItem();
}

void ItemView::insertItem(Plasma::IconWidget *icon, qreal weight)
{
    m_itemContainer->insertItem(icon, weight);
    icon->installEventFilter(this);
}

void ItemView::clear()
{
    m_itemContainer->clear();
}

int ItemView::count() const
{
    return m_itemContainer->count();
}

void ItemView::setOrientation(Qt::Orientation orientation)
{
    m_itemContainer->setOrientation(orientation);
}

Qt::Orientation ItemView::orientation() const
{
    return m_itemContainer->orientation();
}

void ItemView::setIconSize(int size)
{
    m_itemContainer->setIconSize(size);
}

int ItemView::iconSize() const
{
    return m_itemContainer->iconSize();
}

void ItemView::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QRectF rect = boundingRect();
    QPointF newPos(m_itemContainer->pos());
    if (m_itemContainer->size().width() < rect.size().width()) {
        newPos.setX(rect.center().x() - m_itemContainer->size().width()/2);
    } else {
        newPos.setX(qMin(m_itemContainer->pos().x(), (qreal)0.0));
    }
    if (m_itemContainer->size().height() < rect.size().height()) {
        newPos.setY(rect.center().y() - m_itemContainer->size().height()/2);
    } else {
        newPos.setY(qMin(m_itemContainer->pos().y(), (qreal)0.0));
    }
    m_itemContainer->setPos(newPos.toPoint());

    Plasma::ScrollWidget::resizeEvent(event);
}

bool ItemView::eventFilter(QObject *watched, QEvent *event)
{
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);
    if (icon && event->type() == QEvent::GraphicsSceneHoverEnter) {
        if (icon) {
            m_itemContainer->setCurrentItem(icon);
        }
    } else if (watched == m_itemContainer && event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *re = static_cast<QGraphicsSceneResizeEvent *>(event);

        ScrollBarFlags scrollBars = NoScrollBar;
        if (m_itemContainer->pos().x() < 0 || m_itemContainer->geometry().right() > size().width()) {
            scrollBars |= HorizontalScrollBar;
        }
        if (m_itemContainer->pos().y() < 0 || m_itemContainer->geometry().bottom() > size().height()) {
            scrollBars |= VerticalScrollBar;
        }
        emit scrollBarsNeededChanged(scrollBars);

        //FIXME: this is not desired probably?
        if (orientation() == Qt::Horizontal) {
            setMinimumHeight(re->newSize().height());
        }
    //pass click only if the user didn't move the mouse FIXME: we need sendevent there
    } else if(icon && event->type() == QEvent::GraphicsSceneMousePress) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        ScrollWidget::mousePressEvent(me);
    } else if (icon && event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

        ScrollWidget::mouseMoveEvent(me);
    } else if (icon && event->type() == QEvent::GraphicsSceneMouseRelease){
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
        ScrollWidget::mouseReleaseEvent(me);

    } else if (watched == m_itemContainer && event->type() == QEvent::GraphicsSceneMove) {
        QGraphicsSceneMoveEvent *me = static_cast<QGraphicsSceneMoveEvent *>(event);

        ScrollBarFlags scrollBars = NoScrollBar;
        if (m_itemContainer->pos().x() < 0 || m_itemContainer->geometry().right() > size().width()) {
            scrollBars |= HorizontalScrollBar;
        }
        if (m_itemContainer->pos().y() < 0 || m_itemContainer->geometry().bottom() > size().height()) {
            scrollBars |= VerticalScrollBar;
        }
        emit scrollBarsNeededChanged(scrollBars);
    }

    return Plasma::ScrollWidget::eventFilter(watched, event);
}

#include <itemview.moc>
