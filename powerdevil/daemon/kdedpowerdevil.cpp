/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "kdedpowerdevil.h"

#include "powerdevilbackendloader.h"
#include "powerdevilcore.h"

#include <QtCore/QTimer>

#include <KAboutData>
#include <KDebug>
#include <KPluginFactory>

K_PLUGIN_FACTORY( PowerDevilFactory,
                  registerPlugin<KDEDPowerDevil>(); )
K_EXPORT_PLUGIN( PowerDevilFactory( "powerdevildaemon" ) )

#define POWERDEVIL_VERSION "1.99"

KDEDPowerDevil::KDEDPowerDevil(QObject* parent, const QVariantList &)
    : KDEDModule(parent)
{
    QTimer::singleShot(0, this, SLOT(init()));
}

KDEDPowerDevil::~KDEDPowerDevil()
{
}

void KDEDPowerDevil::init()
{
    KGlobal::locale()->insertCatalog("powerdevil");

    KAboutData aboutData("powerdevil", "powerdevil", ki18n("KDE Power Management System"),
                         POWERDEVIL_VERSION, ki18n("KDE Power Management System is PowerDevil, an "
                                                   "advanced, modular and lightweight Power Management "
                                                   "daemon"),
                         KAboutData::License_GPL, ki18n("(c) 2010 MetalWorkers Co."),
                         KLocalizedString(), "http://www.kde.org");

    aboutData.addAuthor(ki18n( "Dario Freddi" ), ki18n("Maintainer"), "drf@kde.org",
                        "http://drfav.wordpress.com");

    PowerDevil::Core *core = new PowerDevil::Core(this, KComponentData(aboutData));

    // Before doing anything, let's set up our backend
    PowerDevil::BackendInterface *interface = PowerDevil::BackendLoader::loadBackend(core);

    if (!interface) {
        // Ouch
        kError() << "KDE Power Management System init failed!";
        core->loadCore(0);
    } else {
        // Let's go!
        kDebug() << "Backend loaded, loading core";
        core->loadCore(interface);
    }
}

#include "kdedpowerdevil.moc"
