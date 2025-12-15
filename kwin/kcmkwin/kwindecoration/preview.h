/*
 *
 * Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KWINDECORATION_PREVIEW_H
#define KWINDECORATION_PREVIEW_H

#include <QWidget>
#include <kdecoration.h>
#include <kdecorationbridge.h>
#include <kdecoration_plugins_p.h>

class KDecorationPreviewBridge;
class KDecorationPreviewOptions;
class QMouseEvent;

class KDecorationPreview
    : public QWidget
{
    Q_OBJECT
public:
    // Note: Windows can't be added or removed without making changes to
    //       the code, since parts of it assume there's just an active
    //       and an inactive window.
    enum Windows { Inactive = 0, Active, NumWindows };

    explicit KDecorationPreview(QWidget* parent = NULL);
    virtual ~KDecorationPreview();
    bool recreateDecoration(KDecorationPlugins* plugin);
    void disablePreview();
    KDecorationFactory *factory() const;
    QRegion unobscuredRegion(bool, const QRegion&) const;
    QRect windowGeometry(bool) const;
    void setTempBorderSize(KDecorationPlugins* plugin, KDecorationDefines::BorderSize size);
    void setTempButtons(KDecorationPlugins* plugin, bool customEnabled, const QString &left, const QString &right);
    QPixmap preview();
    void setMask(const QRegion &region, bool active);
private:
    void render(QPainter *painter, KDecoration *decoration, const QSize &recommendedSize, const QPoint &offset, const QRegion &mask) const;
    KDecorationPreviewOptions* options;
    KDecorationPreviewBridge* bridge[NumWindows];
    KDecoration* deco[NumWindows];
    QRegion m_activeMask;
    QRegion m_inactiveMask;
};

class KDecorationPreviewBridge
    : public KDecorationBridge
{
public:
    KDecorationPreviewBridge(KDecorationPreview* preview, bool active);
    bool isActive() const override;
    bool isCloseable() const override;
    bool isMaximizable() const override;
    MaximizeMode maximizeMode() const override;
    QuickTileMode quickTileMode() const override;
    bool isMinimizable() const override;
    bool providesContextHelp() const override;
    int desktop() const override;
    bool isModal() const override;
    bool isShadeable() const override;
    bool isShade() const override;
    bool isSetShade() const override;
    bool keepAbove() const override;
    bool keepBelow() const override;
    bool isMovable() const override;
    bool isResizable() const override;
    NET::WindowType windowType(unsigned long supported_types) const override;
    QIcon icon() const override;
    QString caption() const override;
    void processMousePressEvent(QMouseEvent*) override;
    void showWindowMenu(const QRect &) override;
    void showWindowMenu(const QPoint &) override;
    void showApplicationMenu(const QPoint &) override;
    bool menuAvailable() const override;
    void performWindowOperation(WindowOperation) override;
    void setMask(const QRegion&, int) override;
    bool isPreview() const override;
    QRect geometry() const override;
    QRect iconGeometry() const override;
    QRegion unobscuredRegion(const QRegion& r) const override;
    WId windowId() const override;
    void closeWindow() override;
    void maximize(MaximizeMode mode) override;
    void minimize() override;
    void showContextHelp() override;
    void setDesktop(int desktop) override;
    void titlebarDblClickOperation() override;
    void titlebarMouseWheelOperation(int delta) override;
    void setShade(bool set) override;
    void setKeepAbove(bool) override;
    void setKeepBelow(bool) override;
    int currentDesktop() const override;
    QWidget* initialParentWidget() const override;
    Qt::WFlags initialWFlags() const override;
    void grabXServer(bool grab) override;

    bool compositingActive() const override;
    QRect transparentRect() const override;

    // Window tabbing
    QString caption(int idx) const override;
    void closeTab(long id) override;
    void closeTabGroup() override;
    long currentTabId() const override;
    QIcon icon(int idx) const override;
    void setCurrentTab(long id) override;
    void showWindowMenu(const QPoint &, long id) override;
    void tab_A_before_B(long A, long B) override;
    void tab_A_behind_B(long A, long B) override;
    int tabCount() const override;
    long tabId(int idx) const override;
    void untab(long id, const QRect& newGeom) override;
    WindowOperation buttonToWindowOperation(Qt::MouseButtons button) override;

private:
    KDecorationPreview* preview;
    bool active;
};

class KDecorationPreviewOptions
    : public KDecorationOptions
{
public:
    KDecorationPreviewOptions();
    virtual ~KDecorationPreviewOptions();
    unsigned long updateSettings();

    void setCustomBorderSize(BorderSize size);
    void setCustomTitleButtonsEnabled(bool enabled);
    void setCustomTitleButtons(const QString &left, const QString &right);

private:
    BorderSize customBorderSize;
    bool customButtonsChanged;
    bool customButtons;
    QString customTitleButtonsLeft;
    QString customTitleButtonsRight;
};

class KDecorationPreviewPlugins
    : public KDecorationPlugins
{
public:
    explicit KDecorationPreviewPlugins(const KSharedConfigPtr &cfg);
    bool provides(Requirement) override;
};

inline KDecorationPreviewPlugins::KDecorationPreviewPlugins(const KSharedConfigPtr &cfg)
    : KDecorationPlugins(cfg)
{
}

#endif
