/* -------------------------------------------------------------

   configdialog.cpp (part of Klipper - Cut & paste history for KDE)

   $Id$

   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Licensed under the Artistic License

 ------------------------------------------------------------- */

#include <qlabel.h>
#include <qlayout.h>
#include <qlistview.h>
#include <qpushbutton.h>
#include <qtooltip.h>
#include <qvbox.h>
#include <qwhatsthis.h>

#include <kaboutdialog.h>
#include <kiconloader.h>
#include <klistview.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kwinmodule.h>

#include "configdialog.h"

ConfigDialog::ConfigDialog( const ActionList *list, KKeyEntryMap *keyMap )
    : KDialogBase( KDialogBase::Tabbed, i18n("Klipper preferences"),
                    KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help,
                    KDialogBase::Ok, 0L, "config dialog" )
{
    QFrame *w = 0L; // the parent for the widgets

    w = addVBoxPage( i18n("&General") );
    generalWidget = new GeneralWidget( w, "general widget" );

    w = addVBoxPage( i18n("Ac&tions") );
    actionWidget = new ActionWidget( list, w, "actions widget" );

    w = addVBoxPage( i18n("&Shortcuts") );
    keysWidget = new KeysWidget( keyMap, w, "shortcuts widget" );


    /* hmmm, this sort of sucks...
    w = addVBoxPage( i18n("About") );
    KAboutWidget *about = new KAboutWidget( w, "about widget" );

    about->setAuthor("Andrew Stanley-Jones", "asj@cban.com",
                     QString::null, QString::null);
    about->addContributor("Carsten Pfeiffer", "pfeiffer@kde.org",
                          QString::null, QString::null);
    about->setMaintainer("Carsten Pfeiffer",
                         QString::null, QString::null, QString::null);
    about->setVersion( "0.7" );
    */
}


ConfigDialog::~ConfigDialog()
{
}

// prevent huge size due to long regexps in the action-widget
void ConfigDialog::show()
{
    static KWinModule module;
    QSize s1 = sizeHint();
    QSize s2 = module.workArea().size();
    int w = isVisible() ? width() : s1.width();
    int h = isVisible() ? height() : s1.height();

    if ( s1.width() >= s2.width() )
	w = s2.width();
    if ( s1.height() >= s2.height() )
	h = s2.height();

    resize( w, h );
    KDialogBase::show();
}

/////////////////////////////////////////
////


GeneralWidget::GeneralWidget( QWidget *parent, const char *name )
    : QVGroupBox( parent, name )
{
    setTitle( i18n("General settings" ));

    cbMousePos = new QCheckBox( i18n("&Popup menu at mouse-cursor position"),
                                this );
    cbSaveContents = new QCheckBox( i18n("Sa&ve clipboard contents on exit"),
                                    this );
    cbReplayAIH = new QCheckBox( i18n("&Replay actions on an item selected from history"),
                                    this );
    // make a QLabel because using popupTimeout->setLabel messes up layout
    QLabel *lblTimeout = new QLabel( i18n("Tim&eout for Action popups:" ), this );
    // workaround for KIntNumInput making a huge QSpinBox
    QHBox *box = new QHBox( this );
    popupTimeout = new KIntNumInput( box );
    lblTimeout->setBuddy( popupTimeout );
    popupTimeout->setRange( 0, 200, 1, false );
//    popupTimeout->setSuffix( i18n(" seconds") );
//    popupTimeout->setLabel( i18n("Timeout for Action popups:"),
//                            AlignVCenter);
    QToolTip::add( popupTimeout, i18n("A value of 0 disables the timeout") );
    QLabel *lblSeconds = new QLabel( "seconds", box );
    box->setStretchFactor( lblSeconds, 10 );
    box->setSpacing(6);
//    QWidget * dummy = new QWidget( box );
//    box->setStretchFactor( dummy, 10 );
    editListBox = new KEditListBox( i18n("&Disable actions for windows of type WM_CLASS:"), this, "editlistbox", true, KEditListBox::Add | KEditListBox::Remove );
    QWhatsThis::add( editListBox,
          i18n("<qt>This lets you specify windows in which klipper should<br>"
	       "not invoke \"actions\". Use"
	       "<center><b>xprop | grep WM_CLASS</b></center>"
	       "in a terminal to find out the WM_CLASS of a window.<br>"
	       "Next, click on the window you want to examine. The<br>"
	       "first string it outputs after the equal sign is the one<br>"
	       "you need to enter here.</qt>"));


#ifdef __GNUC__
#warning Qt Bug, remove these setOrientation lines when fixed
#endif
    setOrientation( Horizontal );
}

GeneralWidget::~GeneralWidget()
{
}


/////////////////////////////////////////
////


ActionWidget::ActionWidget( const ActionList *list, QWidget *parent,
                            const char *name )
    : QVGroupBox( parent, name )
{
    ASSERT( list != 0L );

    setTitle( i18n("Action settings") );

    QLabel *lblAction = new QLabel( "Action &list (right click to add/remove commands):",
				    this );
    listView = new KListView( this, "list view" );
    lblAction->setBuddy( listView );
    listView->addColumn( i18n("Regular expression (see http://doc.trolltech.com/qregexp.html#details)") );
    listView->addColumn( i18n("Description") );

    listView->setRenameable(0);
    listView->setRenameable(1);
    listView->setItemsRenameable( true );
    listView->setItemsMovable( true );
    listView->setAcceptDrops( true );
    listView->setDropVisualizer( true );
    listView->setDragEnabled( true );

    listView->setRootIsDecorated( true );
    listView->setMultiSelection( false );
    listView->setAllColumnsShowFocus( true );
    listView->setSelectionMode( QListView::Single );
    connect( listView, SIGNAL( rightButtonPressed( QListViewItem *,
                                                   const QPoint&, int) ),
             SLOT( slotRightPressed( QListViewItem *, const QPoint&, int )));
    connect( listView,
             SIGNAL( executed( QListViewItem*, const QPoint&, int ) ),
             SLOT( slotItemChanged( QListViewItem*, const QPoint& , int ) ) );

    ClipAction *action   = 0L;
    ClipCommand *command = 0L;
    QListViewItem *item  = 0L;
    QListViewItem *child = 0L;
    QListViewItem *after = 0L; // QListView's default inserting really sucks
    ActionListIterator it( *list );

    const QPixmap& doc = SmallIcon( "misc" );
    const QPixmap& exec = SmallIcon( "exec" );

    for ( action = it.current(); action; action = ++it ) {
        item = new QListViewItem( listView, after,
                                  action->regExp(), action->description() );
        item->setPixmap( 0, doc );

        QListIterator<ClipCommand> it2( action->commands() );
        for ( command = it2.current(); command; command = ++it2 ) {
            child = new QListViewItem( item, after,
                                       command->command, command->description);
        if ( command->pixmap.isEmpty() )
            child->setPixmap( 0, exec );
        else
            child->setPixmap( 0, SmallIcon( command->pixmap ) );
            after = child;
        }
        after = item;
    }

    listView->setSorting( -1 ); // newly inserted items just append unsorted

    QHBox *box = new QHBox( this );
    box->setSpacing( KDialog::spacingHint() );
    QPushButton *button = new QPushButton( i18n("&Add Action"), box );
    connect( button, SIGNAL( clicked() ), SLOT( slotAddAction() ));

    button = new QPushButton( i18n("&Delete Action"), box );
    connect( button, SIGNAL( clicked() ), SLOT( slotDeleteAction() ));

    QLabel *label = new QLabel(i18n("Click on a highlighted item's column to change it. \"%s\" in a command will be replaced with the clipboard contents."), box);
    label->setAlignment( WordBreak | AlignLeft | AlignVCenter );

    box->setStretchFactor( label, 5 );

    setOrientation( Horizontal );
}

ActionWidget::~ActionWidget()
{
}


void ActionWidget::slotRightPressed( QListViewItem *item, const QPoint&, int )
{
    if ( !item )
        return;

    int addCmd = 0, rmCmd = 0;
    KPopupMenu *menu = new KPopupMenu;
    addCmd = menu->insertItem( i18n("Add Command") );
    rmCmd = menu->insertItem( i18n("Remove Command") );
    if ( !item->parent() ) {// no "command" item
        menu->setItemEnabled( rmCmd, false );
        item->setOpen( true );
    }

    int id = menu->exec( QCursor::pos() );
    if ( id == addCmd ) {
        QListViewItem *p = item->parent() ? item->parent() : item;
        QListViewItem *cmdItem = new QListViewItem( p, item,
                         i18n("Click here to set the command to be executed"),
                         i18n("<new command>") );
        cmdItem->setPixmap( 0, SmallIcon( "exec" ) );
    }
    else if ( id == rmCmd )
        delete item;

    delete menu;
}

void ActionWidget::slotItemChanged( QListViewItem *item, const QPoint&, int col )
{
    if ( !item->parent() || col != 0 )
        return;
    ClipCommand command( item->text(0), item->text(1) );
        item->setPixmap( 0, SmallIcon( command.pixmap.isEmpty() ?
                                                   "exec" : command.pixmap ) );
}

void ActionWidget::slotAddAction()
{
    QListViewItem *item = new QListViewItem( listView );
    item->setPixmap( 0, SmallIcon( "misc" ));
    item->setText( 0, i18n("Click here to set the regexp"));
    item->setText( 1, i18n("<new action>"));
}


void ActionWidget::slotDeleteAction()
{
    QListViewItem *item = listView->currentItem();
    if ( item && item->parent() )
        item = item->parent();
    delete item;
}


ActionList * ActionWidget::actionList()
{
    QListViewItem *item = listView->firstChild();
    QListViewItem *child = 0L;
    ClipAction *action = 0L;
    ActionList *list = new ActionList;
    list->setAutoDelete( true );
    while ( item ) {
        action = new ClipAction( item->text( 0 ), item->text( 1 ) );
        child = item->firstChild();

        // add the commands
        while ( child ) {
            action->addCommand( child->text( 0 ), child->text( 1 ), true );
            child = child->nextSibling();
        }

        list->append( action );
        item = item->nextSibling();
    }

    return list;
}


///////////////////////////////////////////////////////
//////////


KeysWidget::KeysWidget( KKeyEntryMap *keyMap, QWidget *parent, const char *name)
    : QVGroupBox( parent, name )
{
    setTitle( i18n("Global keyboard shortcuts") );

    keyChooser = new KKeyChooser( keyMap, this );

    setOrientation( Horizontal );
}

KeysWidget::~KeysWidget()
{
}


#include "configdialog.moc"
