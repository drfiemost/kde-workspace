// $Id$

#include <stdlib.h>
#include "ktablistbox.h"
#include <qfontmet.h>
#include <qpainter.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qapp.h>
#include <qdrawutl.h>
#include <kapp.h>

#define INIT_MAX_ITEMS 16

#include "ktablistbox.moc"


//=============================================================================
//
//  C L A S S   KOldTabListBoxItem
//
//=============================================================================
KOldTabListBoxItem::KOldTabListBoxItem(int aColumns)
{
    columns = aColumns;
    txt = new QString[columns];
    mark = -2;
}


//-----------------------------------------------------------------------------
KOldTabListBoxItem::~KOldTabListBoxItem()
{
    if (txt) delete[] txt;
    txt = NULL;
}


//-----------------------------------------------------------------------------
void KOldTabListBoxItem::setForeground(const QColor& fg)
{
    fgColor = fg;
}


//-----------------------------------------------------------------------------
void KOldTabListBoxItem::setMarked(int m)
{
    mark = m;
}


//-----------------------------------------------------------------------------
KOldTabListBoxItem& KOldTabListBoxItem::operator=(const KOldTabListBoxItem& from)
{
    int i;
    
    for (i=0; i<columns; i++)
        txt[i] = from.txt[i];
    
    fgColor = from.fgColor;
    
    return *this;
}


//=============================================================================
//
//  C L A S S   KOldTabListBoxColumn
//
//=============================================================================
//-----------------------------------------------------------------------------
KOldTabListBoxColumn::KOldTabListBoxColumn(KOldTabListBox* pa, int w): QObject()
{
    initMetaObject();
    iwidth = w;
    colType = KOldTabListBox::TextColumn;
    parent = pa;
}


//-----------------------------------------------------------------------------
KOldTabListBoxColumn::~KOldTabListBoxColumn()
{
}


//-----------------------------------------------------------------------------
void KOldTabListBoxColumn::setWidth(int w)
{
    iwidth = w;
}


//-----------------------------------------------------------------------------
void KOldTabListBoxColumn::setType(KOldTabListBox::ColumnType lbt)
{
    colType = lbt;
}


//-----------------------------------------------------------------------------
void KOldTabListBoxColumn::paintCell(QPainter* paint, int row, 
                                  const QString& string, bool marked)
{
    const QFontMetrics* fm = &paint->fontMetrics();
    QPixmap* pix = NULL;
    int beg, end, x;
    
    // p->fillRect(0, 0, cellWidth(col), cellHeight(row), bg);
    if (marked)
    {
        paint->fillRect(0, 0, iwidth, parent->cellHeight(row), 
                        parent->highlightColor);
    }
    
    switch(colType)
    {
    case KOldTabListBox::PixmapColumn:
        if (string) pix = parent->dict().find(string);
        if (pix)
        {
            paint->drawPixmap(0, 0, *pix);
            break;
        }
        /*else output as string*/
        
    case KOldTabListBox::TextColumn:
        paint->drawText(1, fm->ascent() +(fm->leading()), 
                        (const char*)string); 
        break;
        
    case KOldTabListBox::MixedColumn:
        QString pixName;
        if (!string.isEmpty())
        {
            for (x=0, beg=0; string[beg] == '\t'; x+=parent->tabPixels, beg++);
            end = beg-1;
            
            while(string[beg] == '{')
            {
                end = string.find('}', beg+1);
                if (end >= 0)
                {
                    pixName = string.mid(beg+1, end-beg-1);
                    pix = parent->dict().find(pixName);
                    if (!pix)
                    {
                        warning("KOldTabListBox: no pixmap with name '"+pixName+
                                "' registered. This is a program bug.\n");
                    } 
                    paint->drawPixmap(x, 0, *pix);
                    x += pix->width()+1;
                    beg = end+1;
                }
                else break;
            }
            
            paint->drawText(x+1, fm->ascent() +(fm->leading()), 
                            (const char*)string.mid(beg+1, string.length()-beg+1)); 
        }
        break;
    }
    
    if (marked) 
        paint->fillRect(iwidth-6, 0, iwidth, 128, parent->highlightColor);
    else
        paint->eraseRect(iwidth-6, 0, iwidth, 128);
}


//-----------------------------------------------------------------------------
void KOldTabListBoxColumn::paint(QPainter* paint)
{
    const QFontMetrics* fm = &paint->fontMetrics();
    paint->drawText(3, fm->ascent() +(fm->leading()),(const char*)name());
}




//=============================================================================
//
//   C L A S S   KOldTabListBox
//
//=============================================================================
KOldTabListBox::KOldTabListBox(QWidget *parent, const char *name, int columns,
                         WFlags f): 
KOldTabListBoxInherited(parent, name, f), lbox(this)
{
    const QFontMetrics* fm = &fontMetrics();
    QString f;
    QColorGroup g = colorGroup();
    // KConfig* conf = KApplication::getKApplication()->getConfig();
    
    initMetaObject();
    
    f = kapp->kdedir();
    f.detach();
    f += "/lib/pics/khtmlw_dnd.xpm";
    dndDefaultPixmap.load(f.data());
    
    tabPixels = 10;
    maxItems  = 0;
    current   = -1;
    colList   = NULL;
    itemList  = NULL;
    sepChar   = '\n';
    labelHeight = fm->height() + 4;
    columnPadding = fm->height() / 2;
    highlightColor = g.mid();
    
    lbox.setGeometry(0, labelHeight, width(), height()-labelHeight);
    
    if (columns > 0) setNumCols(columns);
}


//-----------------------------------------------------------------------------
KOldTabListBox::~KOldTabListBox()
{
    if (colList)  delete[] colList;
    if (itemList) delete[] itemList;
    colList  = NULL;
    itemList = NULL;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setNumRows(int aRows)
{
    lbox.setNumRows(aRows);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setTabWidth(int aTabWidth)
{
    tabPixels = aTabWidth;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setNumCols(int aCols)
{
    maxItems = 0;
    
    if (colList) delete[] colList;
    if (itemList) delete[] itemList;
    colList  = NULL;
    itemList = NULL;
    
    if (aCols < 0) aCols = 0;
    lbox.setNumCols(aCols);
    if (aCols <= 0) return;
    
    colList  = new KOldTabListBoxColumn[aCols](this);
    itemList = new KOldTabListBoxItem[INIT_MAX_ITEMS](aCols);
    maxItems = INIT_MAX_ITEMS;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setColumnWidth(int col, int aWidth)
{
    if (col<0 || col>=numCols()) return;
    colList[col].setWidth(aWidth);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setColumn(int col, const char* aName, int aWidth,
                            ColumnType aType)
{
    if (col<0 || col>=numCols()) return;
    
    setColumnWidth(col,aWidth);
    colList[col].setName(aName);
    colList[col].setType(aType);
    update();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setCurrentItem(int idx, int colId)
{
    int i;
    
    if (idx>=numRows()) return;
    
    unmarkAll();
    
    if (idx != current)
    {
        i = current;
        current = idx;
        
        updateItem(i,FALSE);
    }
    
    if (current >= 0)
    {
        markItem(idx);
        emit highlighted(current, colId);
    }
}


//-----------------------------------------------------------------------------
void KOldTabListBox::markItem(int idx, int colId)
{
    if (itemList[idx].marked()==colId) return;
    itemList[idx].setMarked(colId);
    updateItem(idx,FALSE);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::unmarkItem(int idx)
{
    int mark;
    
    mark = itemList[idx].marked();
    itemList[idx].setMarked(-2);
    if (mark>=-1) updateItem(idx);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::unmarkAll(void)
{
    int i;
    
    for (i=numRows()-1; i>=0; i--)
        unmarkItem(i);
}


//-----------------------------------------------------------------------------
const QString& KOldTabListBox::text(int row, int col) const
{
    const KOldTabListBoxItem* item = getItem(row);
    static QString str;
    static QString empty("");
    int i, cols;
    
    if (!item) return empty;
    if (col >= 0) return item->text(col);
    
    cols = item->columns - 1;
    for (str="",i=0; i<=cols; i++)
    {
        str += item->text(i);
        if (i<cols) str += sepChar;
    }
    
    return str;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::insertItem(const char* aStr, int row)
{
    int i;
    
    if (row < 0) row = numRows();
    if (row >= maxItems) resizeList();
    
    for (i=numRows()-1; i>=row; i--)
        itemList[i+1] = itemList[i];
    
    if (current >= row) current++;
    
    setNumRows(numRows()+1);
    changeItem(aStr, row);
    
    if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::changeItem(const char* aStr, int row)
{
    char* str;
    char  sepStr[2];
    char* pos;
    int   i;
    KOldTabListBoxItem* item;
    
    if (row < 0 || row >= numRows()) return;
    
    str = new char[strlen(aStr)+2];
    strcpy(str, aStr);
    
    sepStr[0] = sepChar;
    sepStr[1] = '\0';
    
    item = &itemList[row];
    
    pos = strtok(str, sepStr);
    for (i=0; pos && *pos && i<numCols(); i++)
    {
        item->setText(i, pos);
        pos = strtok(NULL, sepStr);
    }
    item->setForeground(black);
    
    if (needsUpdate(row)) lbox.repaint();
    
    delete str;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::changeItemPart(const char* aStr, int row, int col)
{
    if (row < 0 || row >= numRows()) return;
    if (col < 0 || col >= numCols()) return;
    
    itemList[row].setText(col, aStr);
    if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::changeItemColor(const QColor& newColor, int row)
{
    if (row >= numRows()) return;
    if (row < 0) row = numRows()-1;
    
    itemList[row].setForeground(newColor);
    if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::removeItem(int row)
{
    int i, nr;
    
    if (row < 0 || row >= numRows()) return;
    if (current > row) current--;
    
    nr = numRows()-1;
    for (i=row; i<nr; i++)
        itemList[i] = itemList[i+1];
    
    setNumRows(nr);
    if (nr==0) current = -1;
    
    if (needsUpdate(row)) lbox.repaint();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::updateItem(int row, bool erase)
{
    int i;
    
    for (i=numCols()-1; i>=0; i--)
        lbox.updateCell(row, i, erase);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::clear(void)
{
    int i;
    
    for (i=numRows()-1; i>=0; i--)
        itemList[i].setMarked(-2);
    
    setNumRows(0);
    lbox.setTopLeftCell(0,0);
    current = -1;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::setSeparator(char sep)
{
    sepChar = sep;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::resizeList(int newNumItems)
{
    KOldTabListBoxItem* newItemList;
    int i, ih;
    
    if (newNumItems < 0) newNumItems =(maxItems << 1);
    if (newNumItems < INIT_MAX_ITEMS) newNumItems = INIT_MAX_ITEMS;
    
    newItemList = new KOldTabListBoxItem[newNumItems](numCols());
    
    ih = newNumItems<numRows() ? newNumItems : numRows();
    for (i=ih-1; i>=0; i--)
    {
        newItemList[i] = itemList[i];
    }
    
    delete[] itemList;
    itemList = newItemList;
    maxItems = newNumItems;
    
    setNumRows(ih);
}


//-----------------------------------------------------------------------------
void KOldTabListBox::resizeEvent(QResizeEvent* e)
{
    int i, w;
    
    KOldTabListBoxInherited::resizeEvent(e);
    
    for (i=numCols()-2, w=0; i>=0; i--)
        w += cellWidth(i);
    
    if (w + cellWidth(numCols()-1) < e->size().width())
    {
        setColumnWidth(numCols()-1, e->size().width() - w);
    }
    
    lbox.setGeometry(0, labelHeight, e->size().width(), 
                     e->size().height()-labelHeight);
    lbox.reconnectSBSignals();
    
    repaint();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::paintEvent(QPaintEvent*)
{
    int i, ih, x, w;
    QPainter paint;
    QWMatrix matrix;
    QRect    clipR;
    
    ih = numCols();
    x  = -lbox.xOffset();
    matrix.translate(x, 0);
    
    //if (!isUpdatesEnabled()) return;
    
    paint.begin(this);
    for (i=0; i<ih; i++)
    {
        w = colList[i].width();
        
        if (w + x >= 0)
        {
            clipR.setRect(x, 0, w, labelHeight);
            paint.setWorldMatrix(matrix, FALSE);
            paint.setClipRect(clipR);
            
            colList[i].paint(&paint);
            qDrawShadePanel(&paint, 0, 0, w, labelHeight, 
                            KOldTabListBoxInherited::colorGroup());
        }
        matrix.translate(w, 0);
        x += w;
    }
    paint.end();
}


//-----------------------------------------------------------------------------
bool KOldTabListBox::startDrag(int aCol, int aRow, const QPoint& p)
{
    int       dx = -(dndDefaultPixmap.width() >> 1);
    int       dy = -(dndDefaultPixmap.height() >> 1);
    KDNDIcon* icon = new KDNDIcon(dndDefaultPixmap, p.x()+dx, p.y()+dy);
    int       size, type;
    char*	    data;
    
    if (!prepareForDrag(aCol,aRow, &data, &size, &type)) return FALSE;
    
    KOldTabListBoxInherited::startDrag(icon, data, size, type, dx, dy);
    return TRUE;
}


//-----------------------------------------------------------------------------
bool KOldTabListBox::prepareForDrag(int /*aCol*/, int /*aRow*/, 
                                 char** /*data*/, int* /*size*/,
                                 int* /*type*/)
{
    return FALSE;
}


//-----------------------------------------------------------------------------
void KOldTabListBox::horSbValue(int /*val*/)
{
    update();
}


//-----------------------------------------------------------------------------
void KOldTabListBox::horSbSlidingDone()
{
}




//=============================================================================
//
//   C L A S S   KOldTabListBoxTable
//
//=============================================================================

QPoint KOldTabListBoxTable::dragStartPos;
int KOldTabListBoxTable::dragCol = -1;
int KOldTabListBoxTable::dragRow = -1;
int KOldTabListBoxTable::selIdx  = -1;


KOldTabListBoxTable::KOldTabListBoxTable(KOldTabListBox *parent):
    KOldTabListBoxTableInherited(parent)
{
    QFontMetrics fm = fontMetrics();
    
    initMetaObject();
    
    dragging = FALSE;
    
    setTableFlags(Tbl_autoVScrollBar|Tbl_autoHScrollBar|Tbl_smoothVScrolling|
                  Tbl_clipCellPainting);
    
    switch(style())
    {
    case MotifStyle:
    case WindowsStyle:
        setBackgroundColor(colorGroup().base());
        setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
        break;
    default:
        setLineWidth(1);
        setFrameStyle(QFrame::Panel|QFrame::Plain);
    }
    
    setCellWidth(0);
    setCellHeight(fm.lineSpacing() + 1);
    setNumRows(0);
    
    setFocusPolicy(StrongFocus);
}


//-----------------------------------------------------------------------------
KOldTabListBoxTable::~KOldTabListBoxTable()
{
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::paintCell(QPainter* p, int row, int col)
{
    KOldTabListBox*     owner =(KOldTabListBox*)parentWidget();
    KOldTabListBoxItem* item  = owner->getItem(row);
    
    if (!item) return;
    p->setPen(item->foreground());
    
    owner->colList[col].paintCell(p, row, item->text(col),(item->marked()==-1));
}


//-----------------------------------------------------------------------------
int KOldTabListBoxTable::cellWidth(int col)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    
    return(owner->colList ? owner->colList[col].width() : 0);
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::mouseDoubleClickEvent(QMouseEvent* e)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    int idx, colnr;
    
    //mouseReleaseEvent(event);
    idx = owner->currentItem();
    colnr = findCol(e->pos().x());
    if (idx >= 0) emit owner->selected(idx,colnr);
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::doItemSelection(QMouseEvent* e, int idx)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    int i, di;
    
    owner->unmarkAll();
    if ((e->state()&ShiftButton)!=0 && owner->currentItem()>=0)
    {
        i  = owner->currentItem();
        di =(i>idx ? -1 : 1);
        while(1)
        {
            owner->markItem(i);
            if (i == idx) break;
            i += di;
        }
    }
    else owner->setCurrentItem(idx);
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::mousePressEvent(QMouseEvent* e)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    int row, col;
    
    row = findRow(e->pos().y());
    col = findCol(e->pos().x());
    
    if (e->button() == RightButton)
    {
        // handle popup menu
        if (row >= 0 && col >= 0) emit owner->popupMenu(row, col);
        return;
    }
    else if (e->button() == MidButton) return;
    
    // arm for possible dragging
    dragStartPos = e->pos();
    dragCol = col;
    dragRow = row;
    
    // handle item highlighting
    if (row >= 0 && owner->getItem(row)->marked() < -1)
    {
        doItemSelection(e, row);
        selIdx = row;
    }
    else selIdx = -1;
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::mouseReleaseEvent(QMouseEvent* e)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    int idx;
    
    if (e->button() != LeftButton) return;
    
    if (dragging)
    {
        owner->mouseReleaseEvent(e);
        dragRow = dragCol = -1;
        dragging = FALSE;
    }
    else
    {
        idx = findRow(e->pos().y());
        if (idx >= 0 && selIdx < 0)
            doItemSelection(e, idx);
    }
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::mouseMoveEvent(QMouseEvent* e)
{
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    
    if (dragging)
    {
        owner->mouseMoveEvent(e);
        return;
    }
    
    if ((e->state() &(RightButton|LeftButton|MidButton)) != 0)
    {
        if (dragRow >= 0 && dragCol >= 0 &&
            (abs(e->pos().x()-dragStartPos.x()) >= 5 ||
             abs(e->pos().y()-dragStartPos.y()) >= 5))
        {
            // we have a liftoff !
            dragging = owner->startDrag(dragCol, dragRow, mapToGlobal(e->pos()));
            return;
        }
    }
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::reconnectSBSignals(void)
{
    QWidget* hsb =(QWidget*)horizontalScrollBar();
    KOldTabListBox* owner =(KOldTabListBox*)parentWidget();
    
    if (!hsb) return;
    
    connect(hsb, SIGNAL(valueChanged(int)), owner, SLOT(horSbValue(int)));
    connect(hsb, SIGNAL(sliderReleased()), owner, SLOT(horSbSlidingDone()));
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::focusInEvent(QFocusEvent*)
{
    // Just do nothing here to avoid the annoying flicker whick happens due
    // to a redraw() call per default.
}


//-----------------------------------------------------------------------------
void KOldTabListBoxTable::focusOutEvent(QFocusEvent*)
{
    // Just do nothing here to avoid the annoying flicker whick happens due
    // to a redraw() call per default.
}
