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

#include "itemcontainer.h"
#include "itemview.h"
#include "resultwidget.h"
#include "models/commonmodel.h"
#include "iconactioncollection.h"

#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include <QTimer>
#include <QWidget>
#include <QWeakPointer>
#include <QAbstractItemModel>
#include <QPropertyAnimation>
#include <QAction>

#include <KIconLoader>
#include <KIcon>

#include <Plasma/Applet>
#include <Plasma/IconWidget>
#include <Plasma/ItemBackground>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

ItemContainer::ItemContainer(ItemView *parent)
    : QGraphicsWidget(parent),
      m_orientation(Qt::Vertical),
      m_currentIconIndexX(-1),
      m_currentIconIndexY(-1),
      m_iconSize(KIconLoader::SizeHuge),
      m_maxColumnWidth(1),
      m_maxRowHeight(1),
      m_firstRelayout(true),
      m_dragAndDropMode(ItemContainer::NoDragAndDrop),
      m_dragging(false),
      m_model(0),
      m_itemView(parent)
{
    m_positionAnimation = new QPropertyAnimation(this, "pos", this);
    m_positionAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_positionAnimation->setDuration(250);
    m_layout = new QGraphicsGridLayout(this);

    QGraphicsItem *pi = parent->parentItem();
    Plasma::Applet *applet = 0;
    //FIXME: this way to find the Applet is quite horrible
    while (pi) {
        applet = dynamic_cast<Plasma::Applet *>(pi);
        if (applet) {
            break;
        } else {
            pi = pi->parentItem();
        }
    }
    m_iconActionCollection = new IconActionCollection(applet, this);

    setFocusPolicy(Qt::StrongFocus);
    setAcceptHoverEvents(true);
    m_hoverIndicator = new Plasma::ItemBackground(this);
    m_hoverIndicator->setZValue(-100);
    m_hoverIndicator->hide();

    m_relayoutTimer = new QTimer(this);
    m_relayoutTimer->setSingleShot(true);
    connect(m_relayoutTimer, SIGNAL(timeout()), this, SLOT(relayout()));

    m_setCurrentTimer = new QTimer(this);
    m_setCurrentTimer->setSingleShot(true);
    connect(m_setCurrentTimer, SIGNAL(timeout()), this, SLOT(syncCurrentItem()));
}

ItemContainer::~ItemContainer()
{}

void ItemContainer::setCurrentItem(Plasma::IconWidget *currentIcon)
{
    if (m_relayoutTimer->isActive()) {
        m_setCurrentTimer->start(400);
        m_currentIcon = currentIcon;
        return;
    }

    QWeakPointer<Plasma::IconWidget> currentWeakIcon = currentIcon;
    //m_currentIcon.clear();

    for (int x = 0; x < m_layout->columnCount(); ++x) {
        for (int y = 0; y < m_layout->rowCount(); ++y) {
            if (m_currentIcon.data() == currentIcon) {
                break;
            } if (m_layout->itemAt(y, x) == currentIcon) {
                m_currentIcon = currentIcon;
                m_currentIconIndexX = x;
                m_currentIconIndexY = y;
                emit itemSelected(m_currentIcon.data());
                break;
            }
        }
    }

    m_hoverIndicator->setTargetItem(currentIcon);
}

void ItemContainer::syncCurrentItem()
{
    setCurrentItem(m_currentIcon.data());
}

Plasma::IconWidget *ItemContainer::currentItem() const
{
    return m_currentIcon.data();
}

//FIXME: remove
QList<Plasma::IconWidget *> ItemContainer::items() const
{
    return m_items.values();
}

Plasma::IconWidget *ItemContainer::createItem()
{
    Plasma::IconWidget *item;
    if (!m_usedItems.isEmpty()) {
        item = m_usedItems.last();
        m_usedItems.pop_back();
    } else {
        item = new ResultWidget(this);
        m_itemView->registerAsDragHandle(item);
    }

    item->installEventFilter(m_itemView);
    return item;
}

void ItemContainer::disposeItem(Plasma::IconWidget *icon)
{
    if (m_usedItems.count() < 40) {
        icon->hide();
        icon->removeIconAction(0);
        disconnect(icon, 0, 0, 0);
        m_usedItems.append(icon);
        icon->removeEventFilter(m_itemView);
    } else {
        icon->deleteLater();
    }
}

void ItemContainer::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    if (orientation == Qt::Horizontal) {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    } else {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    }
}

Qt::Orientation ItemContainer::orientation() const
{
    return m_orientation;
}

void ItemContainer::setIconSize(int size)
{
    m_iconSize = size;
}

int ItemContainer::iconSize() const
{
    return m_iconSize;
}

void ItemContainer::setDragAndDropMode(DragAndDropMode mode)
{
    m_dragAndDropMode = mode;
}

ItemContainer::DragAndDropMode ItemContainer::dragAndDropMode() const
{
    return m_dragAndDropMode;
}

void ItemContainer::askRelayout()
{
    m_relayoutTimer->start(500);
}

void ItemContainer::relayout()
{
    if (!m_model) {
        return;
    }

    if (m_layout->count() == 0) {
        hide();
    }

    //Relayout the grid
    int validRow = 0;
    int validColumn = 0;

    QSizeF availableSize;
    QGraphicsWidget *pw = parentWidget();
    //FIXME: if this widget will be scrollwidget::widget itself this part could become a bit prettier
    if (pw) {
        availableSize = pw->size();
    } else {
        availableSize = size();
    }


    if (m_layout->rowCount() > 0 && size().width() <= availableSize.width()) {
        for (int i = 0; i <= m_model->rowCount() - 1; i++) {
            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            Plasma::IconWidget *icon = m_items.value(index);
            const int row = i / m_layout->rowCount();
            const int column = i % m_layout->columnCount();
            if (m_layout->itemAt(row, column) == icon) {
                validRow = row;
                validColumn = column;
            } else {
                break;
            }
        }
    }

    const int nRows = m_layout->rowCount();
    const int nColumns = m_layout->columnCount();
    for (int row = validRow; row < nRows; ++row) {
        for (int column = validColumn; column < nColumns; ++column) {
            QGraphicsLayoutItem * item = m_layout->itemAt(row, column);
            //FIXME: no other way to remove a specific item in a grid layout
            // this s really, really horrible
            for (int i = 0; i < m_layout->count(); ++i) {
                if (m_layout->itemAt(i) == item) {
                    m_layout->removeAt(i);
                    break;
                }
            }
        }
    }

    setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

    //FIXME: here is inefficient but we ore sure only there abut what items are in
    int maxColumnWidth = 0;
    int maxRowHeight = 0;
    foreach (Plasma::IconWidget *icon, m_items) {
        if (icon->size().width() > maxColumnWidth) {
            maxColumnWidth = icon->size().width();
        }
        if (icon->size().height() > maxRowHeight) {
            maxRowHeight = icon->size().height();
        }
    }

    if (m_orientation == Qt::Vertical) {

        int nColumns;
        // if we already decided how many columns are going to be don't decide again
        if (validColumn > 0 && m_layout->columnCount() > 0 &&  m_layout->rowCount() > 0) {
            nColumns = m_layout->columnCount();
        } else {
            nColumns = qMax(1, int(availableSize.width() / maxColumnWidth));
        }

        for (int i = 0; i <= m_model->rowCount() - 1; i++) {
            const int row = i / nColumns;
            const int column = i % nColumns;
            if (m_layout->itemAt(row, column) != 0) {
                continue;
            }

            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            Plasma::IconWidget *icon = m_items.value(index);
            if (icon) {
                m_layout->addItem(icon, row, column);
                m_layout->setAlignment(icon, Qt::AlignHCenter);
                icon->show();
            }
        }
    } else {

        int nRows;
        // if we already decided how many rows are going to be don't decide again
        if (validRow > 0 && m_layout->columnCount() > 0 &&  m_layout->rowCount() > 0) {
            nRows = m_layout->rowCount();
        } else {
            nRows = qMax(1, int(availableSize.height() / maxRowHeight));
        }

        for (int i = 0; i <= m_model->rowCount() - 1; i++) {
            const int row = i % nRows;
            const int column = i / nRows;
            if (m_layout->itemAt(row, column) != 0) {
                continue;
            }

            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            Plasma::IconWidget *icon = m_items.value(index);
            if (icon) {
                m_layout->addItem(icon, row, column);
                m_layout->setAlignment(icon, Qt::AlignCenter);
                icon->show();
            }
        }
    }

    if (!isVisible()) {
        m_layout->activate();
        show();
    }

    const QSizeF newSize = sizeHint(Qt::MinimumSize, QSizeF());

    setMaximumSize(newSize);
    resize(newSize);
    m_relayoutTimer->stop();
    m_firstRelayout = false;
}

void ItemContainer::itemRequestedDrag(Plasma::IconWidget *icon)
{
    if (m_dragging || dragAndDropMode() == NoDragAndDrop) {
        return;
    }

    for (int i = 0; i < m_layout->count(); ++i) {
        if (m_layout->itemAt(i) == icon) {
            if (dragAndDropMode() == MoveDragAndDrop) {
                m_layout->removeAt(i);
                m_dragging = true;
                icon->setZValue(900);
                icon->installEventFilter(this);
                //ugly but necessary to don't make it clipped
                icon->setParentItem(0);
            }

            QModelIndex index = m_model->index(i, 0, m_rootIndex);
            if (index.isValid()) {
                emit dragStartRequested(index);
            }
            return;
        }
    }
}

void ItemContainer::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Left: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexX = (m_layout->columnCount() + m_currentIconIndexX - 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Right: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexX = (m_currentIconIndexX + 1) % m_layout->columnCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Up: {
        m_currentIcon.clear();
        while (!m_currentIcon) {
            m_currentIconIndexY = (m_layout->rowCount() + m_currentIconIndexY - 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Down: {
        m_currentIcon.clear();
        while (!m_currentIcon.data()) {
            m_currentIconIndexY = (m_currentIconIndexY + 1) % m_layout->rowCount();
            m_currentIcon = static_cast<Plasma::IconWidget *>(m_layout->itemAt(m_currentIconIndexY, m_currentIconIndexX));
        }
        m_hoverIndicator->setTargetItem(m_currentIcon.data());
        emit itemSelected(m_currentIcon.data());
        break;
    }
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (m_currentIcon) {
            QModelIndex index = m_itemToIndex.value(m_currentIcon.data());
            if (index.isValid()) {
                emit itemActivated(m_model->index(index.row(), 0, m_rootIndex));
            }
        }
        break;
    case Qt::Key_Backspace:
    case Qt::Key_Home:
        emit resetRequested();
        break;
    default:
        break;
    }
}

QVariant ItemContainer::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionChange) {
        QPointF newPos = value.toPointF();
        if (m_dragging) {
            return pos();
        }
    }

    return QGraphicsWidget::itemChange(change, value);
}

bool ItemContainer::eventFilter(QObject *watched, QEvent *event)
{
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(watched);

    if (event->type() == QEvent::GraphicsSceneMouseMove) {
        QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);

        icon->setPos(icon->mapToParent(me->pos()) - icon->boundingRect().center());
        m_dragging = false;

        m_itemView->setScrollPositionFromDragPosition(icon->mapToParent(me->pos()));
        m_dragging = true;

    } else if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        m_dragging = false;
        icon->setZValue(10);
        icon->removeEventFilter(this);
        icon->setPos(icon->mapToItem(this, QPoint(0,0)));
        icon->setParentItem(this);

        QModelIndex index = m_itemToIndex.value(icon);
        if (index.isValid()) {
            emit itemAskedReorder(index, icon->geometry().center());
        }

        askRelayout();
    }

    return false;
}


int ItemContainer::rowForPosition(const QPointF &point)
{
    //FIXME: this code is ugly as sin and inefficient as well
    //find the two items that will be neighbours
    int row = -1;
    int column = -1;

    QGraphicsLayoutItem *item = 0;
    for (int y = 0; y < m_layout->rowCount(); ++y) {
        item = m_layout->itemAt(y, 0);

        if (item && item->geometry().center().y() < point.y()) {
            row = y;
        }
    }
    if (row == -1 && point.y() > m_layout->geometry().center().y()) {
        row = m_layout->rowCount();
    }

    for (int x = 0; x < m_layout->columnCount(); ++x) {
        item = m_layout->itemAt(0, x);

        if (item && item->geometry().center().x() < point.x()) {
            column = x;
        }
    }
    if (column == -1 && point.x() > m_layout->geometry().center().x()) {
        column = m_layout->columnCount();
    }

    row = qBound(0, row, m_layout->rowCount()-1);

    kDebug() << "The item will be put at" << row << column;

    int modelRow = row*m_layout->columnCount() + qBound(0, column, m_layout->columnCount());

    kDebug() << "Corresponding to the model row" << modelRow;

    return modelRow;
}

void ItemContainer::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (m_layout && m_layout->count() > 0 && m_currentIconIndexX == -1) {
        m_currentIconIndexX = 0;
        m_currentIconIndexY = 0;
        Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(m_layout->itemAt(0, 0));
        emit itemSelected(icon);
        setCurrentItem(icon);
    } else {
        setCurrentItem(m_currentIcon.data());
    }
}

void ItemContainer::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    m_hoverIndicator->hide();
}

void ItemContainer::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event)

    if (!hasFocus()) {
        m_hoverIndicator->hide();
    }
}

void ItemContainer::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event)

    QGraphicsWidget *pw = parentWidget();
    if (pw) {
        QRectF parentRect = pw->boundingRect();
        QPointF newPos(pos());
        if (size().width() < parentRect.size().width()) {
            newPos.setX(parentRect.center().x() - size().width()/2);
        } else {
            newPos.setX(qMin(pos().x(), (qreal)0.0));
        }
        if (size().height() < parentRect.size().height()) {
            newPos.setY(parentRect.center().y() - size().height()/2);
        } else {
            newPos.setY(qMin(pos().y(), (qreal)0.0));
        }
        if (m_positionAnimation->state() == QAbstractAnimation::Running) {
            m_positionAnimation->stop();
        }
        if (m_firstRelayout) {
            setPos(newPos.toPoint());
        } else {
            m_positionAnimation->setEndValue(newPos.toPoint());
            m_positionAnimation->start();
        }
    }

    m_relayoutTimer->start(300);
}

void ItemContainer::setModel(QAbstractItemModel *model)
{
    if (m_model) {
        disconnect(m_model, 0, this, 0);
        reset();
    }

    m_model = model;
    connect (m_model, SIGNAL(modelAboutToBeReset()), this, SLOT(reset()));
    connect (m_model, SIGNAL(rowsInserted(const QModelIndex & , int, int)), this, SLOT(generateItems(const QModelIndex&, int, int)));
    connect (m_model, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(removeItems(const QModelIndex&, int, int)));

    generateItems(m_rootIndex, 0, m_model->rowCount());
}

QAbstractItemModel *ItemContainer::model() const
{
    return m_model;
}

void ItemContainer::reset()
{
    m_hoverIndicator->setTargetItem(0);
    for (int i = 0; i < m_layout->count(); ++i) {
        m_layout->removeAt(0);
    }
    foreach (Plasma::IconWidget *icon, m_items) {
        disposeItem(icon);
    }
    m_items.clear();
    m_itemToIndex.clear();
    askRelayout();
}

void ItemContainer::generateItems(const QModelIndex &parent, int start, int end)
{
    if (parent != m_rootIndex) {
        return;
    }

    for (int i = start; i <= end; i++) {
        QModelIndex index = m_model->index(i, 0, m_rootIndex);

        if (index.isValid()) {
            Plasma::IconWidget *icon = createItem();
            icon->setIcon(index.data(Qt::DecorationRole).value<QIcon>());
            icon->setText(index.data(Qt::DisplayRole).value<QString>());

            Plasma::ToolTipContent toolTipData = Plasma::ToolTipContent();
            toolTipData.setAutohide(true);
            toolTipData.setMainText(index.data(Qt::DisplayRole).value<QString>());
            toolTipData.setSubText(index.data(CommonModel::Description).value<QString>());
            toolTipData.setImage(index.data(Qt::DecorationRole).value<QIcon>());

            Plasma::ToolTipManager::self()->registerWidget(this);
            Plasma::ToolTipManager::self()->setContent(icon, toolTipData);

            CommonModel::ActionType actionType = (CommonModel::ActionType)index.data(CommonModel::ActionTypeRole).value<int>();
            if (actionType != CommonModel::NoAction) {
                QAction *action = new QAction(icon);
                if (actionType == CommonModel::AddAction) {
                    action->setIcon(KIcon("favorites"));
                } else {
                    action->setIcon(KIcon("list-remove"));
                }
                icon->addIconAction(action);
                m_iconActionCollection->addAction(action);
                connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));
            }

            qreal left, top, right, bottom;
            m_hoverIndicator->getContentsMargins(&left, &top, &right, &bottom);
            icon->setContentsMargins(left, top, right, bottom);

            icon->setMinimumSize(icon->sizeFromIconSize(m_iconSize));
            icon->setMaximumSize(icon->sizeFromIconSize(m_iconSize));
            icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            m_items.insert(QPersistentModelIndex(index), icon);
            m_itemToIndex.insert(icon, QPersistentModelIndex(index));
            connect(icon, SIGNAL(clicked()), this, SLOT(itemClicked()));
            connect(icon, SIGNAL(dragStartRequested(Plasma::IconWidget *)), this, SLOT(itemRequestedDrag(Plasma::IconWidget *)));
        }
    }
    m_relayoutTimer->start(500);
}

void ItemContainer::actionTriggered()
{
    Plasma::IconWidget *icon = static_cast<Plasma::IconWidget*>(sender()->parent());
    QModelIndex index = m_itemToIndex.value(icon);

    CommonModel::ActionType actionType = (CommonModel::ActionType)index.data(CommonModel::ActionTypeRole).value<int>();

    if (actionType == CommonModel::RemoveAction) {
        m_model->removeRow(index.row());
    } else if (actionType == CommonModel::AddAction) {
        emit addActionTriggered(index);
    }
}

void ItemContainer::removeItems(const QModelIndex &parent, int start, int end)
{
    if (m_rootIndex != parent) {
        return;
    }

    for (int i = start; i <= end; i++) {
        QModelIndex index = m_model->index(i, 0, m_rootIndex);
        Plasma::IconWidget *icon = m_items.value(index);
        m_items.remove(index);
        m_itemToIndex.remove(icon);
        disposeItem(icon);
    }
    m_relayoutTimer->start(500);
}

void ItemContainer::itemClicked()
{
    Plasma::IconWidget *icon = qobject_cast<Plasma::IconWidget *>(sender());

    if (icon) {
        QModelIndex i = m_itemToIndex.value(icon);
        if (i.isValid()) {
            QModelIndex index = m_model->index(i.row(), 0, m_rootIndex);
            emit itemActivated(index);
        }
    }
}

void ItemContainer::setRootIndex(QModelIndex index)
{
    m_rootIndex = index;
    reset();
}

QModelIndex ItemContainer::rootIndex() const
{
    return m_rootIndex;
}


#include <itemcontainer.moc>
