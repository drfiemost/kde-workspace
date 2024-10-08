/*
 *   Copyright 2008 Aaron Seigo <aseigo@kde.org>
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

#include "desktopcorona.h"

#include <QAction>
#include <QApplication>
#include <QDir>
#include <QGraphicsLayout>
#include <QTimer>
#include <QMenu>
#include <QSignalMapper>

#include <KAction>
#include <KDebug>
#include <KDialog>
#include <KGlobal>
#include <KGlobalSettings>
#include <KServiceTypeTrader>
#include <KStandardDirs>
#include <KSycoca>
#include <KWindowSystem>

#include <Plasma/AbstractToolBox>
#include <Plasma/Containment>
#include <plasma/containmentactionspluginsconfig.h>
#include <Plasma/Context>
#include <Plasma/DataEngineManager>
#include <Plasma/Package>

#include <kephal/screens.h>

#include <scripting/layouttemplatepackagestructure.h>

#include "panelview.h"
#include "plasmaapp.h"
#include "plasma-shell-desktop.h"
#include "scripting/desktopscriptengine.h"

static const QString s_panelTemplatesPath("plasma-layout-templates/panels/*");

DesktopCorona::DesktopCorona(QObject *parent)
    : Plasma::Corona(parent),
      m_addPanelAction(0),
      m_addPanelsMenu(0),
      m_delayedUpdateTimer(new QTimer(this))
{
    init();
}

DesktopCorona::~DesktopCorona()
{
    delete m_addPanelsMenu;
}

void DesktopCorona::init()
{
    setPreferredToolBoxPlugin(Plasma::Containment::DesktopContainment, "org.kde.desktoptoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::CustomContainment, "org.kde.desktoptoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::PanelContainment, "org.kde.paneltoolbox");
    setPreferredToolBoxPlugin(Plasma::Containment::CustomPanelContainment, "org.kde.paneltoolbox");

    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init start" << "(line:" << __LINE__ << ")";
    Kephal::Screens *screens = Kephal::Screens::self();
    connect(screens, SIGNAL(screenAdded(Kephal::Screen*)), SLOT(screenAdded(Kephal::Screen*)));
    connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), this, SIGNAL(availableScreenRegionChanged()));

    Plasma::ContainmentActionsPluginsConfig desktopPlugins;
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::Vertical, "switchdesktop");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::MidButton, "paste");
    desktopPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");
    Plasma::ContainmentActionsPluginsConfig panelPlugins;
    panelPlugins.addPlugin(Qt::NoModifier, Qt::RightButton, "contextmenu");

    setContainmentActionsDefaults(Plasma::Containment::DesktopContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomContainment, desktopPlugins);
    setContainmentActionsDefaults(Plasma::Containment::PanelContainment, panelPlugins);
    setContainmentActionsDefaults(Plasma::Containment::CustomPanelContainment, panelPlugins);

    checkAddPanelAction();
    connect(this, SIGNAL(immutabilityChanged(Plasma::ImmutabilityType)),
            this, SLOT(updateImmutability(Plasma::ImmutabilityType)));
    connect(KSycoca::self(), SIGNAL(databaseChanged(QStringList)), this, SLOT(checkAddPanelAction(QStringList)));
    // Everything will be repainted shortly after a screen resize was processed, but unfortunately that does not suffice:
    // When screen size is increased, parts of the newly uncovered area on the panel remain black for some reason. Also,
    // if compositing is disabled and the desktop background is a color, parts of the panel are left on the background.
    // Redrawing the entire scene shortly thereafter works around the problem.
    m_delayedUpdateTimer->setSingleShot(true);
    m_delayedUpdateTimer->setInterval(250); // 100ms was not enough
    connect(this, SIGNAL(availableScreenRegionChanged()), m_delayedUpdateTimer, SLOT(start()));
    connect(m_delayedUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));

    mapAnimation(Plasma::Animator::AppearAnimation, Plasma::Animator::ZoomAnimation);
    mapAnimation(Plasma::Animator::DisappearAnimation, Plasma::Animator::ZoomAnimation);
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "DesktopCorona init end" << "(line:" << __LINE__ << ")";
}

void DesktopCorona::checkAddPanelAction(const QStringList &sycocaChanges)
{
    if (!sycocaChanges.isEmpty() && !sycocaChanges.contains("services")) {
        return;
    }

    delete m_addPanelAction;
    m_addPanelAction = 0;

    delete m_addPanelsMenu;
    m_addPanelsMenu = 0;

    KPluginInfo::List panelContainmentPlugins = Plasma::Containment::listContainmentsOfType("panel");
    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'panel' ~in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);

    if (panelContainmentPlugins.count() + templates.count() == 1) {
        m_addPanelAction = new QAction(i18n("Add Panel"), this);
        m_addPanelAction->setData(Plasma::AbstractToolBox::AddTool);
        connect(m_addPanelAction, SIGNAL(triggered(bool)), this, SLOT(addPanel()));
    } else if (!panelContainmentPlugins.isEmpty()) {
        m_addPanelsMenu = new QMenu;
        m_addPanelAction = m_addPanelsMenu->menuAction();
        m_addPanelAction->setText(i18n("Add Panel"));
        m_addPanelAction->setData(Plasma::AbstractToolBox::AddTool);
        kDebug() << "populateAddPanelsMenu" << panelContainmentPlugins.count();
        connect(m_addPanelsMenu, SIGNAL(aboutToShow()), this, SLOT(populateAddPanelsMenu()));
        connect(m_addPanelsMenu, SIGNAL(triggered(QAction*)), this, SLOT(addPanel(QAction*)));
    }

    if (m_addPanelAction) {
        m_addPanelAction->setIcon(KIcon("list-add"));
        addAction("add panel", m_addPanelAction);
    }
}

void DesktopCorona::updateImmutability(Plasma::ImmutabilityType immutability)
{
    if (m_addPanelAction) {
        m_addPanelAction->setEnabled(immutability == Plasma::Mutable);
    }
}

void DesktopCorona::checkScreens(bool signalWhenExists)
{
    // quick sanity check to ensure we have containments for each screen
    int num = numScreens();
    for (int i = 0; i < num; ++i) {
        checkScreen(i, signalWhenExists);
    }
}

void DesktopCorona::checkScreen(int screen, bool signalWhenExists)
{
    // signalWhenExists is there to allow PlasmaApp to know when to create views
    // it does this only on containment addition, but in the case of a screen being
    // added and the containment already existing for that screen, no signal is emitted
    // and so PlasmaApp does not know that it needs to create a view for it. to avoid
    // taking care of that case in PlasmaApp (which would duplicate some of the code below,
    // DesktopCorona will, when signalWhenExists is true, emit a containmentAdded signal
    // even if the containment actually existed prior to this method being called.
    //
    //note: hte signal actually triggers view creation only for panels, atm.
    //desktop views are created in response to containment's screenChanged signal instead, which is
    //buggy (sometimes the containment thinks it's already on the screen, so no view is created)
    //ensure the panels get views too
    if (signalWhenExists) {
        foreach (Plasma::Containment * c, containments()) {
            if (c->screen() != screen) {
                continue;
            }

            Plasma::Containment::Type t = c->containmentType();
            if (t == Plasma::Containment::PanelContainment ||
                t == Plasma::Containment::CustomPanelContainment) {
                emit containmentAdded(c);
            }
        }
    }
}

int DesktopCorona::numScreens() const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that there is only one screen
        return 1;
    }
#endif

    return Kephal::ScreenUtils::numScreens();
}

QRect DesktopCorona::screenGeometry(int id) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that screen 0 is the default screen, in fact, we pretend
        // we have only one screen at all
        Display *dpy = XOpenDisplay(NULL);
        if (dpy) {
            id = DefaultScreen(dpy);
            XCloseDisplay(dpy);
        }
    }
#endif

    return Kephal::ScreenUtils::screenGeometry(id);
}

QRegion DesktopCorona::availableScreenRegion(int id) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that screen 0 is the default screen, in fact, we pretend
        // we have only one screen at all
        Display *dpy = XOpenDisplay(NULL);
        if (dpy) {
            id = DefaultScreen(dpy);
            XCloseDisplay(dpy);
        }
    }
#endif

    if (id < 0) {
        id = Kephal::ScreenUtils::primaryScreenId();
    }

    QRegion r(screenGeometry(id));
    foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
        if (view->screen() == id && view->visibilityMode() == PanelView::NormalPanel) {
            r = r.subtracted(view->geometry());
        }
    }

    return r;
}

QRect DesktopCorona::availableScreenRect(int id) const
{
    if (id < 0) {
        id = Kephal::ScreenUtils::primaryScreenId();
    }

    QRect r(screenGeometry(id));

    foreach (PanelView *view, PlasmaApp::self()->panelViews()) {
        if (view->screen() == id && view->visibilityMode() == PanelView::NormalPanel) {
            QRect v = view->geometry();
            switch (view->location()) {
                case Plasma::TopEdge:
                    if (v.bottom() > r.top()) {
                        r.setTop(v.bottom());
                    }
                    break;

                case Plasma::BottomEdge:
                    if (v.top() < r.bottom()) {
                        r.setBottom(v.top());
                    }
                    break;

                case Plasma::LeftEdge:
                    if (v.right() > r.left()) {
                        r.setLeft(v.right());
                    }
                    break;

                case Plasma::RightEdge:
                    if (v.left() < r.right()) {
                        r.setRight(v.left());
                    }
                    break;

                default:
                    break;
            }
        }
    }

    return r;
}

int DesktopCorona::screenId(const QPoint &pos) const
{
#ifdef Q_WS_X11
    if (KGlobalSettings::isMultiHead()) {
        // with multihead, we "lie" and say that there is only one screen
        return 0;
    }
#endif

    return Kephal::ScreenUtils::screenId(pos);
}

void DesktopCorona::processUpdateScripts()
{
    evaluateScripts(WorkspaceScripting::ScriptEngine::pendingUpdateScripts());
}

void DesktopCorona::evaluateScripts(const QStringList &scripts, bool isStartup)
{
    foreach (const QString &script, scripts) {
        WorkspaceScripting::DesktopScriptEngine scriptEngine(this, isStartup);
        connect(&scriptEngine, SIGNAL(printError(QString)), this, SLOT(printScriptError(QString)));
        connect(&scriptEngine, SIGNAL(print(QString)), this, SLOT(printScriptMessage(QString)));
        connect(&scriptEngine, SIGNAL(createPendingPanelViews()), PlasmaApp::self(), SLOT(createWaitingPanels()));

        QFile file(script);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
            QString code = file.readAll();
            kDebug() << "evaluating startup script:" << script;
            scriptEngine.evaluateScript(code);
        }
    }
}

void DesktopCorona::printScriptError(const QString &error)
{
    kWarning() << "Startup script error:" << error;
}

void DesktopCorona::printScriptMessage(const QString &error)
{
    kDebug() << "Startup script: " << error;
}

void DesktopCorona::loadDefaultLayout()
{
    evaluateScripts(WorkspaceScripting::ScriptEngine::defaultLayoutScripts());
    if (containments().isEmpty()) {
        QString defaultConfig = KStandardDirs::locate("appdata", "plasma-default-layoutrc");
        if (!defaultConfig.isEmpty()) {
            kDebug() << "attempting to load the default layout from:" << defaultConfig;
            loadLayout(defaultConfig);
            QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
        }
    }

    QTimer::singleShot(1000, this, SLOT(saveDefaultSetup()));
}

void DesktopCorona::saveDefaultSetup()
{
    // a "null" KConfigGroup is used to force a save into the config file
    KConfigGroup invalidConfig;

    foreach (Plasma::Containment *containment, containments()) {
        containment->save(invalidConfig);
        foreach (Plasma::Applet* applet, containment->applets()) {
            applet->save(invalidConfig);
        }
    }

    requestConfigSync();
}

Plasma::Applet *DesktopCorona::loadDefaultApplet(const QString &pluginName, Plasma::Containment *c)
{
    QVariantList args;
    Plasma::Applet *applet = Plasma::Applet::load(pluginName, 0, args);

    if (applet) {
        c->addApplet(applet);
    }

    return applet;
}

void DesktopCorona::screenAdded(Kephal::Screen *s)
{
    kDebug() << s->id();
    checkScreen(s->id(), true);
}

void DesktopCorona::populateAddPanelsMenu()
{
    m_addPanelsMenu->clear();
    const KPluginInfo emptyInfo;

    KPluginInfo::List panelContainmentPlugins = Plasma::Containment::listContainmentsOfType("panel");
    QMap<QString, QPair<KPluginInfo, KService::Ptr> > sorted;
    foreach (const KPluginInfo &plugin, panelContainmentPlugins) {
        //FIXME: a better way to filter out what is not wanted?
        if (!plugin.property("X-Plasma-ContainmentCategories").value<QStringList>().contains("netbook")) {
            sorted.insert(plugin.name(), qMakePair(plugin, KService::Ptr(0)));
        }
    }

    const QString constraint = QString("[X-Plasma-Shell] == '%1' and 'panel' in [X-Plasma-ContainmentCategories]")
                                      .arg(KGlobal::mainComponent().componentName());
    KService::List templates = KServiceTypeTrader::self()->query("Plasma/LayoutTemplate", constraint);
    foreach (const KService::Ptr &service, templates) {
        sorted.insert(service->name(), qMakePair(emptyInfo, service));
    }

    QMapIterator<QString, QPair<KPluginInfo, KService::Ptr> > it(sorted);
    Plasma::PackageStructure::Ptr templateStructure(new WorkspaceScripting::LayoutTemplatePackageStructure);
    while (it.hasNext()) {
        it.next();
        QPair<KPluginInfo, KService::Ptr> pair = it.value();
        if (pair.first.isValid()) {
            KPluginInfo plugin = pair.first;
            QAction *action = m_addPanelsMenu->addAction(plugin.name());
            if (!plugin.icon().isEmpty()) {
                action->setIcon(KIcon(plugin.icon()));
            }

            action->setData(plugin.pluginName());
        } else {
            //FIXME: proper names
            KPluginInfo info(pair.second);
            const QString path = KStandardDirs::locate("data", templateStructure->defaultPackageRoot() + '/' + info.pluginName() + '/');
            if (!path.isEmpty()) {
                Plasma::Package package(path, templateStructure);
                const QString scriptFile = package.filePath("mainscript");
                if (!scriptFile.isEmpty()) {
                    QAction *action = m_addPanelsMenu->addAction(info.name());
                    action->setData(QString::fromLatin1("plasma-desktop-template:%1").arg(scriptFile));
                }
            }
        }
    }
}

void DesktopCorona::addPanel()
{
    KPluginInfo::List panelPlugins = Plasma::Containment::listContainmentsOfType("panel");

    if (!panelPlugins.isEmpty()) {
        addPanel(panelPlugins.first().pluginName());
    }
}

void DesktopCorona::addPanel(QAction *action)
{
    const QString plugin = action->data().toString();
    if (plugin.startsWith("plasma-desktop-template:")) {
        evaluateScripts(QStringList() << plugin.right(plugin.length() - qstrlen("plasma-desktop-template:")), false);
    } else if (!plugin.isEmpty()) {
        addPanel(plugin);
    }
}

void DesktopCorona::addPanel(const QString &plugin)
{
    Plasma::Containment *panel = addContainment(plugin);
    if (!panel) {
        return;
    }

    panel->showConfigurationInterface();

    //Fall back to the cursor position since we don't know what is the originating containment
    const int screen = Kephal::ScreenUtils::screenId(QCursor::pos());

    panel->setScreen(screen);

    QList<Plasma::Location> freeEdges = DesktopCorona::freeEdges(screen);

    //kDebug() << freeEdges;
    Plasma::Location destination;
    if (freeEdges.contains(Plasma::BottomEdge)) {
        destination = Plasma::BottomEdge;
    } else if (freeEdges.contains(Plasma::TopEdge)) {
        destination = Plasma::TopEdge;
    } else if (freeEdges.contains(Plasma::LeftEdge)) {
        destination = Plasma::LeftEdge;
    } else if (freeEdges.contains(Plasma::RightEdge)) {
        destination = Plasma::RightEdge;
    } else destination = Plasma::TopEdge;

    panel->setLocation(destination);

    const QRect screenGeom = screenGeometry(screen);
    const QRegion availGeom = availableScreenRegion(screen);
    int minH = 10;
    int minW = 10;
    int w = 35;
    int h = 35;

    //FIXME: this should really step through the rects on the relevant screen edge to find
    //appropriate space
    if (destination == Plasma::LeftEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.y(), w, screenGeom.height())).boundingRect();
        h = r.height();
        minW = 35;
        minH = h;
    } else if (destination == Plasma::RightEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.right() - w, screenGeom.y(), w, screenGeom.height())).boundingRect();
        h = r.height();
        minW = 35;
        minH = h;
    } else if (destination == Plasma::TopEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.y(), screenGeom.width(), h)).boundingRect();
        w = r.width();
        minH = 35;
        minW = w;
    } else if (destination == Plasma::BottomEdge) {
        QRect r = availGeom.intersected(QRect(screenGeom.x(), screenGeom.bottom() - h, screenGeom.width(), h)).boundingRect();
        w = r.width();
        minH = 35;
        minW = w;
    }

    panel->setMinimumSize(minW, minH);
    panel->setMaximumSize(w, h);
    panel->resize(w, h);
}

#include "desktopcorona.moc"

