//
// KDE Shortcut config module
//
// Copyright (c)  Mark Donohoe 1998
// Copyright (c)  Matthias Ettrich 1998
// Converted to generic key configuration module, Duncan Haldane 1998.

#ifndef __KEYCONFIG_H__
#define __KEYCONFIG_H__

#include <qpushbutton.h>
#include <qlistbox.h>

#include <kaccel.h>
#include <kkeydialog.h>
#include <kcmodule.h>
#include <qdict.h>
#include "savescm.h"

class KeyChooserSpec;

class KKeyModule : public KCModule
{
	Q_OBJECT
public:
	KAccel *keys;
        KKeyEntryMap dict;
        KeyChooserSpec *kc;

	KKeyModule( QWidget *parent, bool isGlobal, const char *name = 0 );
	~KKeyModule ();

        void load();
        void save();
        void defaults();
        void init();

public slots:
	void slotPreviewScheme( int );
	void slotAdd();
	void slotSave();
	void slotRemove();
	void slotChanged();
        void updateKeys( const KKeyEntryMap* map_P );

signals:
        void keysChanged( const KKeyEntryMap* map_P );

protected:
	QListBox *sList;
	QStringList *sFileList;
	QPushButton *addBt;
	QPushButton *removeBt;
	int nSysSchemes;

	void readSchemeNames();
	void readScheme( int index=0 );

        QString KeyType ;
        QString KeyScheme ;
        QString KeySet ;

};

class KeyChooserSpec : public KKeyChooser
{
        Q_OBJECT
public:
        KeyChooserSpec( KKeyEntryMap *aKeyDict, QWidget* parent,
                 bool global );
        void updateKeys( const KKeyEntryMap* map_P );
protected:
        bool global;
};

#endif

