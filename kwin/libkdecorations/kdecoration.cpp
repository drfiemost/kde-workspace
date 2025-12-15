/*****************************************************************
This file is part of the KDE project.

Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
******************************************************************/

#include "kdecoration.h"
#include "kdecoration_p.h"

#include <kdebug.h>
#include <QApplication>
#include <QMenu>
#include <kglobal.h>
#include <assert.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#include <QX11Info>

#include "kdecorationfactory.h"
#include "kdecorationbridge.h"


/*

Extending KDecoration:
======================

If KDecoration will ever need to be extended in a way that'd break binary compatibility
(i.e. adding new virtual methods most probably), new class KDecoration2 should be
inherited from KDecoration and those methods added there. Code that would depend
on the new functionality could then dynamic_cast<> to KDecoration2 to check whether
it is available and use it.

KCommonDecoration would have to be extended the same way, adding KCommonDecoration2
inheriting KCommonDecoration and adding the new API matching KDecoration2.

*/

class KDecorationPrivate
{
public:
    KDecorationPrivate(KDecorationBridge *b, KDecorationFactory *f)
        : bridge(b)
        , factory(f)
        , alphaEnabled(false)
        , w()
    {
    }
    KDecorationBridge *bridge;
    KDecorationFactory *factory;
    bool alphaEnabled;
    QScopedPointer<QWidget> w;
};

KDecoration::KDecoration(KDecorationBridge* bridge, KDecorationFactory* factory)
    :   d(new KDecorationPrivate(bridge, factory))
{
    factory->addDecoration(this);
}

KDecoration::~KDecoration()
{
    factory()->removeDecoration(this);
    delete d;
}

const KDecorationOptions* KDecoration::options()
{
    return KDecorationOptions::self();
}

void KDecoration::createMainWidget(Qt::WFlags flags)
{
    // FRAME check flags?
    QWidget *w = new QWidget(initialParentWidget(), initialWFlags() | flags);
    w->setObjectName(QLatin1String("decoration widget"));
    w->setAttribute(Qt::WA_PaintOnScreen);
    if (options()->showTooltips())
        w->setAttribute(Qt::WA_AlwaysShowToolTips);
    setMainWidget(w);
}

void KDecoration::setMainWidget(QWidget* w)
{
    assert(d->w.isNull());
    d->w.reset(w);
    w->setMouseTracking(true);
    widget()->resize(geometry().size());
}

QWidget* KDecoration::initialParentWidget() const
{
    return d->bridge->initialParentWidget();
}

Qt::WFlags KDecoration::initialWFlags() const
{
    return d->bridge->initialWFlags();
}

bool KDecoration::isActive() const
{
    return d->bridge->isActive();
}

bool KDecoration::isCloseable() const
{
    return d->bridge->isCloseable();
}

bool KDecoration::isMaximizable() const
{
    return d->bridge->isMaximizable();
}

KDecoration::MaximizeMode KDecoration::maximizeMode() const
{
    return d->bridge->maximizeMode();
}

KDecoration::QuickTileMode KDecoration::quickTileMode() const
{
    return d->bridge->quickTileMode();
}

bool KDecoration::isMinimizable() const
{
    return d->bridge->isMinimizable();
}

bool KDecoration::providesContextHelp() const
{
    return d->bridge->providesContextHelp();
}

int KDecoration::desktop() const
{
    return d->bridge->desktop();
}

bool KDecoration::isModal() const
{
    return d->bridge->isModal();
}

bool KDecoration::isShadeable() const
{
    return d->bridge->isShadeable();
}

bool KDecoration::isShade() const
{
    return d->bridge->isShade();
}

bool KDecoration::isSetShade() const
{
    return d->bridge->isSetShade();
}

bool KDecoration::keepAbove() const
{
    return d->bridge->keepAbove();
}

bool KDecoration::keepBelow() const
{
    return d->bridge->keepBelow();
}

bool KDecoration::isMovable() const
{
    return d->bridge->isMovable();
}

bool KDecoration::isResizable() const
{
    return d->bridge->isResizable();
}

NET::WindowType KDecoration::windowType(unsigned long supported_types) const
{
    // this one is also duplicated in KDecorationFactory
    return d->bridge->windowType(supported_types);
}

QIcon KDecoration::icon() const
{
    return d->bridge->icon();
}

QString KDecoration::caption() const
{
    return d->bridge->caption();
}

void KDecoration::processMousePressEvent(QMouseEvent* e)
{
    return d->bridge->processMousePressEvent(e);
}

void KDecoration::showWindowMenu(const QRect &pos)
{
    d->bridge->showWindowMenu(pos);
}

void KDecoration::showWindowMenu(QPoint pos)
{
    d->bridge->showWindowMenu(pos);
}

void KDecoration::showApplicationMenu(const QPoint &p)
{
    d->bridge->showApplicationMenu(p);
}

bool KDecoration::menuAvailable() const
{
    return d->bridge->menuAvailable();
}

void KDecoration::performWindowOperation(WindowOperation op)
{
    d->bridge->performWindowOperation(op);
}

void KDecoration::setMask(const QRegion& reg, int mode)
{
    d->bridge->setMask(reg, mode);
}

void KDecoration::clearMask()
{
    d->bridge->setMask(QRegion(), 0);
}

bool KDecoration::isPreview() const
{
    return d->bridge->isPreview();
}

QRect KDecoration::geometry() const
{
    return d->bridge->geometry();
}

QRect KDecoration::iconGeometry() const
{
    return d->bridge->iconGeometry();
}

QRegion KDecoration::unobscuredRegion(const QRegion& r) const
{
    return d->bridge->unobscuredRegion(r);
}

WId KDecoration::windowId() const
{
    return d->bridge->windowId();
}

void KDecoration::closeWindow()
{
    d->bridge->closeWindow();
}

void KDecoration::maximize(Qt::MouseButtons button)
{
    performWindowOperation(options()->operationMaxButtonClick(button));
}

void KDecoration::maximize(MaximizeMode mode)
{
    d->bridge->maximize(mode);
}

void KDecoration::minimize()
{
    d->bridge->minimize();
}

void KDecoration::showContextHelp()
{
    d->bridge->showContextHelp();
}

void KDecoration::setDesktop(int desktop)
{
    d->bridge->setDesktop(desktop);
}

void KDecoration::toggleOnAllDesktops()
{
    if (isOnAllDesktops())
        setDesktop(d->bridge->currentDesktop());
    else
        setDesktop(NET::OnAllDesktops);
}

void KDecoration::titlebarDblClickOperation()
{
    d->bridge->titlebarDblClickOperation();
}

void KDecoration::titlebarMouseWheelOperation(int delta)
{
    d->bridge->titlebarMouseWheelOperation(delta);
}

void KDecoration::setShade(bool set)
{
    d->bridge->setShade(set);
}

void KDecoration::setKeepAbove(bool set)
{
    d->bridge->setKeepAbove(set);
}

void KDecoration::setKeepBelow(bool set)
{
    d->bridge->setKeepBelow(set);
}

bool KDecoration::drawbound(const QRect&, bool)
{
    return false;
}

bool KDecoration::windowDocked(Position)
{
    return false;
}

void KDecoration::reset(unsigned long)
{
}

void KDecoration::grabXServer()
{
    d->bridge->grabXServer(true);
}

void KDecoration::ungrabXServer()
{
    d->bridge->grabXServer(false);
}

KDecoration::Position KDecoration::mousePosition(const QPoint& p) const
{
    const int range = 16;
    int bleft, bright, btop, bbottom;
    borders(bleft, bright, btop, bbottom);
    btop = std::min(btop, 4);   // otherwise whole titlebar would have resize cursor

    Position m = PositionCenter;

    if ((p.x() > bleft && p.x() < widget()->width() - bright)
            && (p.y() > btop && p.y() < widget()->height() - bbottom))
        return PositionCenter;

    if (p.y() <= std::max(range, btop) && p.x() <= std::max(range, bleft))
        m = PositionTopLeft;
    else if (p.y() >= widget()->height() - std::max(range, bbottom)
            && p.x() >= widget()->width() - std::max(range, bright))
        m = PositionBottomRight;
    else if (p.y() >= widget()->height() - std::max(range, bbottom) && p.x() <= std::max(range, bleft))
        m = PositionBottomLeft;
    else if (p.y() <= std::max(range, btop) && p.x() >= widget()->width() - std::max(range, bright))
        m = PositionTopRight;
    else if (p.y() <= btop)
        m = PositionTop;
    else if (p.y() >= widget()->height() - bbottom)
        m = PositionBottom;
    else if (p.x() <= bleft)
        m = PositionLeft;
    else if (p.x() >= widget()->width() - bright)
        m = PositionRight;
    else
        m = PositionCenter;
    return m;
}

QRect KDecoration::transparentRect() const
{
    return d->bridge->transparentRect();
}

void KDecoration::setAlphaEnabled(bool enabled)
{
    if (d->alphaEnabled == enabled) {
        return;
    }
    d->alphaEnabled = enabled;
    emit alphaEnabledChanged(enabled);
}

bool KDecoration::isAlphaEnabled() const
{
    return d->alphaEnabled;
}

bool KDecoration::compositingActive() const
{
    return d->bridge->compositingActive();
}

void KDecoration::padding(int &left, int &right, int &top, int &bottom) const
{
    left = right = top = bottom = 0;
}

//BEGIN Window tabbing

int KDecoration::tabCount() const
{
    return d->bridge->tabCount();
}

long KDecoration::tabId(int idx) const
{
    return d->bridge->tabId(idx);
}

QString KDecoration::caption(int idx) const
{
    return d->bridge->caption(idx);
}

QIcon KDecoration::icon(int idx) const
{
    return d->bridge->icon(idx);
}

long KDecoration::currentTabId() const
{
    return d->bridge->currentTabId();
}

void KDecoration::setCurrentTab(long id)
{
    d->bridge->setCurrentTab(id);
}

void KDecoration::tab_A_before_B(long A, long B)
{
    d->bridge->tab_A_before_B(A, B);
}

void KDecoration::tab_A_behind_B(long A, long B)
{
    d->bridge->tab_A_behind_B(A, B);
}

void KDecoration::untab(long id, const QRect& newGeom)
{
    d->bridge->untab(id, newGeom);
}

void KDecoration::closeTab(long id)
{
    d->bridge->closeTab(id);
}

void KDecoration::closeTabGroup()
{
    d->bridge->closeTabGroup();
}

void KDecoration::showWindowMenu(const QPoint &pos, long id)
{
    d->bridge->showWindowMenu(pos, id);
}

//END tabbing

KDecoration::WindowOperation KDecoration::buttonToWindowOperation(Qt::MouseButtons button)
{
    return d->bridge->buttonToWindowOperation(button);
}

QRegion KDecoration::region(KDecorationDefines::Region)
{
    return QRegion();
}

KDecorationDefines::Position KDecoration::titlebarPosition()
{
    return PositionTop;
}

QWidget* KDecoration::widget()
{
    return d->w.data();
}

const QWidget* KDecoration::widget() const
{
    return d->w.data();
}

KDecorationFactory* KDecoration::factory() const
{
    return d->factory;
}

bool KDecoration::isOnAllDesktops() const
{
    return desktop() == NET::OnAllDesktops;
}

int KDecoration::width() const
{
    return geometry().width();
}

int KDecoration::height() const
{
    return geometry().height();
}

QString KDecorationDefines::tabDragMimeType()
{
    return "text/ClientGroupItem";
}

KDecorationOptions* KDecorationOptions::s_self = nullptr;

KDecorationOptions::KDecorationOptions()
    : d(new KDecorationOptionsPrivate)
{
    assert(s_self == nullptr);
    s_self = this;
}

KDecorationOptions::~KDecorationOptions()
{
    assert(s_self == this);
    s_self = nullptr;
    delete d;
}

KDecorationOptions* KDecorationOptions::self()
{
    return s_self;
}

QColor KDecorationOptions::color(ColorType type, bool active) const
{
    return(d->colors[type + (active ? 0 : NUM_COLORS)]);
}

QFont KDecorationOptions::font(bool active, bool small) const
{
    if (small)
        return(active ? d->activeFontSmall : d->inactiveFontSmall);
    else
        return(active ? d->activeFont : d->inactiveFont);
}

QPalette KDecorationOptions::palette(ColorType type, bool active) const
{
    int idx = type + (active ? 0 : NUM_COLORS);
    if (d->pal[idx])
        return(*d->pal[idx]);
    // TODO: Why construct the palette this way? Is it worth porting to Qt4?
    //d->pal[idx] = new QPalette(Qt::black, d->colors[idx], d->colors[idx].light(150),
    //                           d->colors[idx].dark(), d->colors[idx].dark(120),
    //                           Qt::black, QApplication::palette().active().
    //                           base());
    d->pal[idx] = new QPalette(d->colors[idx]);
    return(*d->pal[idx]);
}

unsigned long KDecorationOptions::updateSettings(KConfig* config)
{
    return d->updateSettings(config);
}

bool KDecorationOptions::customButtonPositions() const
{
    return d->custom_button_positions;
}

QString KDecorationOptions::titleButtonsLeft() const
{
    return d->title_buttons_left;
}

QString KDecorationOptions::defaultTitleButtonsLeft()
{
    return "M";
}

QString KDecorationOptions::titleButtonsRight() const
{
    return d->title_buttons_right;
}

QString KDecorationOptions::defaultTitleButtonsRight()
{
    return "IAX";
}

bool KDecorationOptions::showTooltips() const
{
    return d->show_tooltips;
}

KDecorationOptions::BorderSize KDecorationOptions::preferredBorderSize(KDecorationFactory* factory) const
{
    assert(factory != nullptr);
    if (d->cached_border_size == BordersCount)   // invalid
        d->cached_border_size = d->findPreferredBorderSize(d->border_size,
                                factory->borderSizes());
    return d->cached_border_size;
}

bool KDecorationOptions::moveResizeMaximizedWindows() const
{
    // TODO KF5: remove function with API break
    return false;
}

KDecorationDefines::WindowOperation KDecorationOptions::operationMaxButtonClick(Qt::MouseButtons button) const
{
    return button == Qt::RightButton ? d->opMaxButtonRightClick :
           button == Qt::MidButton ?   d->opMaxButtonMiddleClick :
           d->opMaxButtonLeftClick;
}

void KDecorationOptions::setOpMaxButtonLeftClick(WindowOperation op)
{
    d->opMaxButtonLeftClick = op;
}

void KDecorationOptions::setOpMaxButtonRightClick(WindowOperation op)
{
    d->opMaxButtonRightClick = op;
}

void KDecorationOptions::setOpMaxButtonMiddleClick(WindowOperation op)
{
    d->opMaxButtonMiddleClick = op;
}

void KDecorationOptions::setBorderSize(BorderSize bs)
{
    d->border_size = bs;
    d->cached_border_size = BordersCount; // invalid
}

void KDecorationOptions::setCustomButtonPositions(bool b)
{
    d->custom_button_positions = b;
}

void KDecorationOptions::setTitleButtonsLeft(const QString& b)
{
    d->title_buttons_left = b;
}

void KDecorationOptions::setTitleButtonsRight(const QString& b)
{
    d->title_buttons_right = b;
}

extern "C" {

int decoration_bridge_version()
{
    return KWIN_DECORATION_BRIDGE_API_VERSION;
}

}

#include "kdecoration.moc"
