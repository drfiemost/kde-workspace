/*****************************************************************
 KWin - the KDE window manager
 This file is part of the KDE project.

Copyright (C) 2007 Rivo Laks <rivolaks@hot.ee>

You can Freely distribute this program under the GNU General Public
License. See the file "COPYING" for the exact licensing terms.
******************************************************************/


#ifndef ADVANCED_H
#define ADVANCED_H

#include <kdialog.h>

#include "ui_advanced.h"


namespace KWin
{

class KWinAdvancedCompositingOptions : public KDialog
{
    Q_OBJECT
    public:
        KWinAdvancedCompositingOptions(QWidget* parent, KSharedConfigPtr config);
        virtual ~KWinAdvancedCompositingOptions();

        void load();

    public slots:
        void changed();
        void save();

    signals:
        void configSaved();

    private:
        KSharedConfigPtr mKWinConfig;
        Ui::KWinAdvancedCompositingOptions ui;
};

} // namespace

#endif
