/*
 * Copyright (c) 2007      Gustavo Pichorim Boiko <gustavo.boiko@kdemail.net>
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KActionCollection>
#include <KApplication>
#include <KCMultiDialog>
#include <KComponentData>
#include <KHelpMenu>
#include <KIcon>
#include <KIconLoader>
#include <KLocale>
#include <KMenu>
#include <QMouseEvent>
#include <QVariant>

#include "krandrtray.h"
#include "krandrpassivepopup.h"
#include "krandrtray.moc"
#include "legacyrandrscreen.h"
#include "randrscreen.h"
#include "randroutput.h"
#include "randrmode.h"

KRandRSystemTray::KRandRSystemTray(QWidget* parent)
	: KSystemTrayIcon(parent)
	, m_popupUp(false)
	, m_help(new KHelpMenu(parent, KGlobal::mainComponent().aboutData(), false, actionCollection()))
{
	setIcon(KSystemTrayIcon::loadIcon("randr"));
	connect(this, SIGNAL(quitSelected()), kapp, SLOT(quit()));
	this->setToolTip( i18n("Screen resize & rotate"));
	connect(this, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(slotActivated(QSystemTrayIcon::ActivationReason)));	

        slotPrepareMenu();

#ifdef HAS_RANDR_1_2
	for (int i = 0; i < numScreens(); ++i)
		connect(screen(i), SIGNAL(configChanged()), this, SLOT(slotPrepareMenu()));
#endif

}

void KRandRSystemTray::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger)
		contextMenu()->show();
}

void KRandRSystemTray::slotPrepareMenu()
{
	QAction *action;
        KMenu *menu = new KMenu(parentWidget());
        setContextMenu(menu);

	menu->clear();

	if (!isValid()) {
		action = menu->addAction(i18n("Required X Extension Not Available"));
		action->setEnabled(false);

	} else {
		m_screenPopups.clear();
		for (int s = 0; s < numScreens(); s++) {
			setCurrentScreen(s);
			if (s == screenIndexOfWidget(parentWidget())) {
				/*lastIndex = menu->insertItem(i18n("Screen %1").arg(s+1));
				menu->setItemEnabled(lastIndex, false);*/
			} else {
#ifdef HAS_RANDR_1_2
			    if (RandR::has_1_2)
				    currentScreen()->loadSettings();
#endif
					
                            KMenu* subMenu = new KMenu(i18n("Screen %1", s+1), menu );
                            subMenu->setObjectName( QString("screen%1").arg(s+1) );
                            m_screenPopups.append(subMenu);
                            populateMenu(subMenu);
                            action = menu->addMenu(subMenu);
                            connect(subMenu, SIGNAL(activated(int)), SLOT(slotScreenActivated()));
			}
		}

		setCurrentScreen(screenIndexOfWidget(parentWidget()));
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			currentScreen()->loadSettings();
#endif
		populateMenu(menu);
	}

	menu->addSeparator();

	QAction *actPrefs = actionCollection()->addAction( QString() );
        actPrefs->setIcon( KIcon( "configure" ) );
        actPrefs->setText( i18n( "Configure Display..." ) );
        connect( actPrefs, SIGNAL( triggered( bool ) ), SLOT( slotPrefs() ) );
	menu->addAction( actPrefs );

	menu->addMenu(/*SmallIcon("help-contents"),KStandardGuiItem::help().text(),*/ m_help->menu());
	QAction *quitAction = actionCollection()->action(KStandardAction::name(KStandardAction::Quit));
	menu->addAction( quitAction );
}

void KRandRSystemTray::slotScreenActivated()
{
	setCurrentScreen(m_screenPopups.indexOf(static_cast<KMenu*>(sender())));
}

void KRandRSystemTray::configChanged()
{
	refresh();

	static bool first = true;

	if (!first)
	{
		QString message;
#ifdef HAS_RANDR_1_2
		if (RandR::has_1_2)
			// TODO: display config changed message
			message = "Screen config changed";
		else
#endif
			message = currentLegacyScreen()->changedMessage();

		KRandrPassivePopup::message(
		i18n("Screen configuration has changed"),
		message, SmallIcon("view-fullscreen"),
		parentWidget());
	}

	first = false;
}

void KRandRSystemTray::populateMenu(KMenu* menu)
{
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2) {
		QAction *action;

		//TODO: populate the menu with the required items
		OutputMap outputs = currentScreen()->outputs();
		if (outputs.count() <= 0)
			return;

		RandRScreen *screen = currentScreen();
		Q_ASSERT(screen);

		menu->addTitle(SmallIcon("view-fullscreen"), i18n("Outputs"));
		OutputMap::const_iterator it = outputs.constBegin();
		while (it != outputs.constEnd()) {
			if (it.value()->isConnected()) {
				RandROutput *output = it.value();
				Q_ASSERT(output);

				KMenu *outputMenu = new KMenu(output->name());
				outputMenu->setIcon(SmallIcon(output->icon()));
				outputMenu->addTitle(SmallIcon("view-fullscreen"), i18n("Screen Size"));

				SizeList sizes = output->sizes();
				QSize currentSize = output->currentSize();
				QActionGroup *sizeGroup = new QActionGroup(outputMenu);
				for (int i = 0; i < sizes.count(); ++i) {
					QSize size = sizes[i];
					action = outputMenu->addAction(QString("%1 x %2").arg(size.width()).arg(size.height()));
					action->setData(size);
					sizeGroup->addAction(action);
					if (size == currentSize) {
						QFont font = action->font();
						font.setBold(true);
						action->setFont(font);
					}

				}
				connect(sizeGroup, SIGNAL(triggered(QAction*)), output, SLOT(slotChangeSize(QAction*)));
				
				action = outputMenu->addAction(i18n("Disable"));
				if (output->currentCrtc() == None)
				{
					QFont font = action->font();
					font.setBold(true);
					action->setFont(font);
				}
				connect(action, SIGNAL(triggered(bool)), output, SLOT(slotDisable()));

				// Display the rotations
				int rotations = output->rotations(), currentRotation = output->currentRotation();
				// Don't display the rotation options if there is no point (ie. none are supported)
				// XFree86 4.3 does not include rotation support.
				if (rotations != RandR::Rotate0) {
					outputMenu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));

					QActionGroup *rotateGroup = new QActionGroup(outputMenu);
					for (int i = 0; i < 6; i++) {
						if ((1 << i) & rotations) {
							action = outputMenu->addAction(QIcon(RandR::rotationIcon(1 << i, currentRotation)), 
											   RandR::rotationName(1 << i));

							action->setData(1 << i);
							if (currentRotation & (1 << i))
							{
								QFont font = action->font();
								font.setBold(true);
								action->setFont(font);
							}
							rotateGroup->addAction(action);
						}
					}
					connect(rotateGroup, SIGNAL(triggered(QAction*)), output, SLOT(slotChangeRotation(QAction*)));
				}
				
				menu->addMenu(outputMenu);
			} else {
				action = menu->addAction(SmallIcon(it.value()->icon()), it.value()->name());
				action->setEnabled(false);
			}
			++it;
		}
	}
	else
#endif
		populateLegacyMenu(menu);
}

void KRandRSystemTray::populateLegacyMenu(KMenu* menu)
{
	QAction *action;

	menu->addTitle(SmallIcon("view-fullscreen"), i18n("Screen Size"));

	LegacyRandRScreen *screen = currentLegacyScreen();
	Q_ASSERT(screen);

	int numSizes = screen->numSizes();
	int* sizeSort = new int[numSizes];

	for (int i = 0; i < numSizes; i++) {
		sizeSort[i] = screen->pixelCount(i);
	}

	QActionGroup *screenSizeGroup = new QActionGroup(menu);
	for (int j = 0; j < numSizes; j++) {
		int highest = -1, highestIndex = -1;

		for (int i = 0; i < numSizes; i++) {
			if (sizeSort[i] && sizeSort[i] > highest) {
				highest = sizeSort[i];
				highestIndex = i;
			}
		}
		sizeSort[highestIndex] = -1;
		Q_ASSERT(highestIndex != -1);

		action = menu->addAction(i18n("%1 x %2", screen->pixelSize(highestIndex).width(), screen->pixelSize(highestIndex).height()));

		if (screen->proposedSize() == highestIndex)
			action->setChecked(true);

		action->setData(highestIndex);
		screenSizeGroup->addAction(action);
	}
	connect(screenSizeGroup, SIGNAL(triggered(QAction*)), SLOT(slotResolutionChanged(QAction*)));
	delete [] sizeSort;
	sizeSort = 0L;

	// Don't display the rotation options if there is no point (ie. none are supported)
	// XFree86 4.3 does not include rotation support.
	if (screen->rotations() != RandR::Rotate0) {
		menu->addTitle(SmallIcon("view-refresh"), i18n("Orientation"));

		QActionGroup *rotationGroup = new QActionGroup(menu);
		for (int i = 0; i < 6; i++) {
			if ((1 << i) & screen->rotations()) {
				action = menu->addAction(QIcon(RandR::rotationIcon(1 << i, screen->currentRotation())), 
							  RandR::rotationName(1 << i));

				if (screen->proposedRotation() & (1 << i))
					action->setChecked(true);

				action->setData(1 << i);
				rotationGroup->addAction(action);
			}
		}
		connect(rotationGroup, SIGNAL(triggered(QAction*)), SLOT(slotOrientationChanged(QAction*)));
	}

	QStringList rr = screen->refreshRates(screen->proposedSize());

	if (rr.count())
		menu->addTitle(SmallIcon("clock"), i18n("Refresh Rate"));

	int i = 0;
	QActionGroup *rateGroup = new QActionGroup(menu);
	for (QStringList::Iterator it = rr.begin(); it != rr.end(); ++it, i++) {
		action = menu->addAction(*it);

		if (screen->proposedRefreshRate() == i)
			action->setChecked(true);

		action->setData(i);
		rateGroup->addAction(action);
	}
	connect(rateGroup, SIGNAL(triggered(QAction*)), SLOT(slotRefreshRateChanged(QAction*)));
}

void KRandRSystemTray::slotResolutionChanged(QAction *action)
{
	Q_ASSERT(action);
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: this will become screen size changed, but for now it is ok
		currentScreenIndex();
	else
#endif
	{
		int index = action->data().toInt();
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);

		if (screen->currentSize() == index)
			return;

		screen->proposeSize(index);

		screen->proposeRefreshRate(-1);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotOrientationChanged(QAction *action)
{
	Q_ASSERT(action);
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: I guess this won't be used in randr1.2, but later we'll see
		currentScreenIndex();
	else
#endif
	{
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);
		
		int propose = screen->currentRotation();
		int rotate = action->data().toInt();

		if (rotate & RandR::RotateMask)
			propose &= RandR::ReflectMask;

		propose ^= rotate;

		if (screen->currentRotation() == propose)
			return;

		screen->proposeRotation(propose);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotRefreshRateChanged(QAction *action)
{
	Q_ASSERT(action);
#ifdef HAS_RANDR_1_2
	if (RandR::has_1_2)
		//TODO: this will probably have to change as refresh rate is differect for 
		//      each crtc
		currentScreenIndex();
	else
#endif
	{
		LegacyRandRScreen *screen = currentLegacyScreen();
		Q_ASSERT(screen);
		
		int index = action->data().toInt();

		if (screen->currentRefreshRate() == index)
			return;

		screen->proposeRefreshRate(index);

		if (screen->applyProposedAndConfirm()) {
			KConfig config("kcmrandrrc");
			if (syncTrayApp(config))
				screen->save(config);
		}
	}
}

void KRandRSystemTray::slotPrefs()
{
	KCMultiDialog *kcm = new KCMultiDialog( parentWidget() );
	kcm->setFaceType( KCMultiDialog::Plain );
	kcm->setPlainCaption( i18n( "Configure Display" ) );
	kcm->addModule( "display" );
	kcm->exec();
}
