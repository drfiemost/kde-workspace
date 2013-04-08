/********************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 1999, 2000    Daniel M. Duley <mosfet@kde.org>
Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

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

#include "decorations.h"
#include "config-kwin.h"

#include <kglobal.h>
#include <KDE/KLocalizedString>
#include <stdlib.h>
#include <QPixmap>

namespace KWin
{

KWIN_SINGLETON_FACTORY(DecorationPlugin)

DecorationPlugin::DecorationPlugin(QObject *)
    : KDecorationPlugins(KGlobal::config())
    , m_noDecoration(false)
{
    defaultPlugin = "kwin3_oxygen";
#ifndef KWIN_BUILD_OXYGEN
    defaultPlugin = "kwin3_aurorae";
#endif
#ifdef KWIN_BUILD_DECORATIONS
    loadPlugin("");   // load the plugin specified in cfg file
#else
    setNoDecoration(true);
#endif
}

DecorationPlugin::~DecorationPlugin()
{
    s_self = NULL;
}

void DecorationPlugin::error(const QString &error_msg)
{
    qWarning("%s", QString(i18n("KWin: ") + error_msg).toLocal8Bit().data());

    setNoDecoration(true);
}

bool DecorationPlugin::provides(Requirement)
{
    return false;
}

void DecorationPlugin::setNoDecoration(bool noDecoration)
{
    m_noDecoration = noDecoration;
}

bool DecorationPlugin::hasNoDecoration() const
{
    return m_noDecoration;
}

} // namespace