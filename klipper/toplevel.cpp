/* -------------------------------------------------------------

   toplevel.cpp (part of Klipper - Cut & paste history for KDE)

   (C) by Andrew Stanley-Jones
   (C) 2000 by Carsten Pfeiffer <pfeiffer@kde.org>

   Generated with the KDE Application Generator

 ------------------------------------------------------------- */

#include <qclipboard.h>
#include <qcursor.h>
#include <qfile.h>
#include <qintdict.h>
#include <qpainter.h>
#include <qtooltip.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kkeydialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ksimpleconfig.h>
#include <kstringhandler.h>
#include <kwin.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <dcopclient.h>
#include <kiconloader.h>
#include <khelpmenu.h>

#include "configdialog.h"
#include "toplevel.h"
#include "urlgrabber.h"
#include "version.h"


#define QUIT_ITEM    50
#define CONFIG_ITEM  60
#define EMPTY_ITEM   80

#define MENU_ITEMS   (( isApplet() ? 6 : 8 ) + ( bTearOffHandle ? 1 : 0 ))
// the <clipboard empty> item
#define EMPTY (m_popup->count() - MENU_ITEMS)


TopLevel::TopLevel( QWidget *parent, bool applet )
    : KSystemTray( parent ), DCOPObject( "klipper" ), m_dcop( 0 )
{
    clip = kapp->clipboard();
    m_selectedItem = -1;

    if( applet ) {
        m_config = new KConfig( "klipperrc" );
        // if there's klipper process running, quit it
        QByteArray arg1, arg2;
        QCString str;
        // call() - wait for finishing
        kapp->dcopClient()->call("klipper", "klipper", "quitProcess()", arg1, str, arg2 );
        // register ourselves, so if klipper process is started,
        // it will quit immediately (KUniqueApplication)
        m_dcop = new DCOPClient;
        m_dcop->registerAs( "klipper", false );
    }
    else
        m_config = kapp->config();

    QSempty = i18n("<empty clipboard>");

    bTearOffHandle = KGlobalSettings::insertTearOffHandle();

    // we need that collection, otherwise KToggleAction is not happy :}
    KActionCollection *collection = new KActionCollection( this, "my collection" );
    toggleURLGrabAction = new KToggleAction( collection, "toggleUrlGrabAction" );
    toggleURLGrabAction->setEnabled( true );

    myURLGrabber = 0L;
    KConfig *kc = m_config;
    readConfiguration( kc );
    setURLGrabberEnabled( bURLGrabber );

    m_lastString = "";
    m_popup = new KPopupMenu(0L, "main_menu");
    connect(m_popup, SIGNAL(activated(int)), SLOT(clickedMenu(int)));

    readProperties(m_config);
    connect(kapp, SIGNAL(saveYourself()), SLOT(saveSession()));

    m_checkTimer = new QTimer(this, "timer");
    m_checkTimer->start(1000, FALSE);
    connect(m_checkTimer, SIGNAL(timeout()), this, SLOT(newClipData()));
    connect( clip, SIGNAL( selectionChanged() ), SLOT(slotSelectionChanged()));
    connect( clip, SIGNAL( dataChanged() ), SLOT( slotClipboardChanged() ));

    // do NOT use UserIcon or appdata or something like that -- this breaks in
    // the kicker applet case!
    m_pixmap = QPixmap( locate( "data", "klipper/pics/klipper_dock.png"  ));
    adjustSize();

    globalKeys = new KGlobalAccel(this);
    KGlobalAccel* keys = globalKeys;
#include "klipperbindings.cpp"
    globalKeys->readSettings(m_config);
    globalKeys->updateConnections();
    toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

    connect( toggleURLGrabAction, SIGNAL( toggled( bool )), 
             this, SLOT( setURLGrabberEnabled( bool )));

    QToolTip::add( this, i18n("Klipper - Clipboard Tool") );
}

TopLevel::~TopLevel()
{
    delete m_checkTimer;
    delete m_popup;
    delete myURLGrabber;
    if( isApplet()) {
        delete m_config;
        delete m_dcop;
    }
}

void TopLevel::adjustSize()
{
    resize( m_pixmap.size() );
}

// this is used for quiting klipper process, if klipper is being started as an applet
void TopLevel::quitProcess()
{
    if( !isApplet()) {
        kapp->dcopClient()->detach();
        kapp->quit();
    }
}

// this is just to make klipper process think we're KUniqueApplication
int TopLevel::newInstance()
{
    return 0;
}

QString TopLevel::getClipboardContents()
{
    return clipboardContents();
}

void TopLevel::setClipboardContents(QString s)
{
    setClipboard( s, Clipboard | Selection);
    newClipData();
}

void TopLevel::clearClipboardContents()
{
    slotClearClipboard();	
}

void TopLevel::mousePressEvent(QMouseEvent *e)
{
    if ( e->button() == LeftButton || e->button() == RightButton )
        showPopupMenu( m_popup );
}

void TopLevel::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    int x = (width() - m_pixmap.width()) / 2;
    int y = (height() - m_pixmap.height()) / 2;
    if ( x < 0 ) x = 0;
    if ( y < 0 ) y = 0;
    p.drawPixmap(x , y, m_pixmap);
    p.end();
}

void TopLevel::clickedMenu(int id)
{
    switch ( id ) {
    case -1:
        break;
    case CONFIG_ITEM:
        slotConfigure();
        break;
    case QUIT_ITEM: {
        saveSession();
        int autoStart = KMessageBox::questionYesNoCancel( 0L, i18n("Should Klipper start automatically\nwhen you login?"), i18n("Automatically Start Klipper?") );

        KConfig *config = KGlobal::config();
        config->setGroup("General");
        if ( autoStart == KMessageBox::Yes )
            config->writeEntry("AutoStart", true);
        else if ( autoStart == KMessageBox::No) {
            config->writeEntry("AutoStart", false);
        }else  // cancel chosen don't quit
	    break;
	config->sync();
        kapp->quit();
        break;
        }
//    case URLGRAB_ITEM: // handled with an extra slot
//	break;
    case EMPTY_ITEM:
	if ( !bClipEmpty )
	{
	    m_checkTimer->stop();

	    trimClipHistory(0);
            slotClearClipboard();
            setEmptyClipboard();

	    m_checkTimer->start(1000);
	}
	break;
    default:
	if ( id == URLGrabItem )
	{
	    break; // handled by its own slot
	}
	else if ( !bClipEmpty )
	{
	    m_checkTimer->stop();
	    //CT mark up the currently put into clipboard -
            // so that user can see later
            if ( m_selectedItem != -1 )
                m_popup->setItemChecked(m_selectedItem, false);

	    m_selectedItem = id;
	    m_popup->setItemChecked(m_selectedItem, true);
            QMapIterator<long,QString> it = m_clipDict.find( id );

            if ( it != m_clipDict.end() && it.data() != QSempty )
            {
                QString data = it.data();
                setClipboard( data, Clipboard | Selection );

		if (bURLGrabber && bReplayActionInHistory)
		    myURLGrabber->checkNewData( data );

                m_lastString = data;

                // We want to move the just selected item to the top of the popup
                // menu. But when we do this right here, we get a crash a little
                // bit later. So instead, we fire a timer to perform the moving.
                QTimer::singleShot( 0, this, SLOT( slotMoveSelectedToTop() ));
            }

	    m_checkTimer->start(1000);
	}
    }
}


void TopLevel::showPopupMenu( QPopupMenu *menu )
{
    Q_ASSERT( menu != 0L );

    menu->move(-1000,-1000);
    menu->show();
    menu->hide();

    if (bPopupAtMouse) {
        QPoint g = QCursor::pos();
        if ( menu->height() < g.y() )
            menu->popup(QPoint( g.x(), g.y() - menu->height()));
        else
            menu->popup(QPoint(g.x(), g.y()));
    }

    else {
        KWin::Info i = KWin::info( winId() );
        QRect g = i.geometry;
	QRect screen = QApplication::desktop()->screenGeometry(QApplication::desktop()->screenNumber(g.center()));

        if ( g.x()-screen.x() > screen.width()/2 &&
             g.y()-screen.y() + menu->height() > screen.height() )
            menu->popup(QPoint( g.x(), g.y() - menu->height()));
        else
            menu->popup(QPoint( g.x() + width(), g.y() + height()));

        //      menu->exec(mapToGlobal(QPoint( width()/2, height()/2 )));
    }
}


void TopLevel::readProperties(KConfig *kc)
{
  QStringList dataList;

  m_popup->clear();
  m_popup->insertTitle( SmallIcon( "klipper" ),
                        i18n("Klipper - Clipboard Tool"));

  if (bKeepContents) { // load old clipboard if configured
      KConfigGroupSaver groupSaver(kc, "General");
      dataList = kc->readListEntry("ClipboardData");

      for (QStringList::ConstIterator it = dataList.begin();
           it != dataList.end(); ++it)
      {
          QString data( *it );
          data.replace( QRegExp( "&" ), "&&" );

          long id = m_popup->insertItem( KStringHandler::csqueeze(data, 45), 
                  -2, -1);
          m_clipDict.insert( id, *it );
      }
  }

  bClipEmpty = clipboardContents().simplifyWhiteSpace().isEmpty() &&
               dataList.isEmpty();

  m_popup->insertSeparator();
  toggleURLGrabAction->plug( m_popup, -1 );
  URLGrabItem = m_popup->idAt( m_popup->count() - 1 );

  m_popup->insertItem( SmallIcon("history_clear"),
			i18n("&Clear Clipboard History"), EMPTY_ITEM );
  m_popup->insertItem( SmallIcon("configure"), i18n("&Preferences..."),
                       CONFIG_ITEM);

  KHelpMenu *help = new KHelpMenu( this, KGlobal::instance()->aboutData(), 
            false );
  m_popup->insertItem( i18n( "&Help" ), help->menu() );

  if( !isApplet()) {
    m_popup->insertSeparator();
    m_popup->insertItem(SmallIcon("exit"), i18n("&Quit"), QUIT_ITEM );
  }
  if( bTearOffHandle )
    m_popup->insertTearOffHandle();

  if (bClipEmpty)
      setEmptyClipboard();
}


void TopLevel::readConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    bPopupAtMouse = kc->readBoolEntry("PopupAtMousePosition", false);
    bKeepContents = kc->readBoolEntry("KeepClipboardContents", true);
    bURLGrabber = kc->readBoolEntry("URLGrabberEnabled", true);
    bReplayActionInHistory = kc->readBoolEntry("ReplayActionInHistory", false);
    bNoNullClipboard = kc->readBoolEntry("NoEmptyClipboard", true);
    bUseGUIRegExpEditor = kc->readBoolEntry("UseGUIRegExpEditor", true );
    maxClipItems = kc->readNumEntry("MaxClipItems", 7);
}

void TopLevel::writeConfiguration( KConfig *kc )
{
    kc->setGroup("General");
    kc->writeEntry("PopupAtMousePosition", bPopupAtMouse);
    kc->writeEntry("KeepClipboardContents", bKeepContents);
    kc->writeEntry("ReplayActionInHistory", bReplayActionInHistory);
    kc->writeEntry("NoEmptyClipboard", bNoNullClipboard);
    kc->writeEntry("UseGUIRegExpEditor", bUseGUIRegExpEditor);
    kc->writeEntry("MaxClipItems", maxClipItems);
    kc->writeEntry("Version", klipper_version );

    if ( myURLGrabber )
        myURLGrabber->writeConfiguration( kc );

    kc->sync();
}

// save session on shutdown. Don't simply use the c'tor, as that may not be called.
void TopLevel::saveSession()
{
  if ( bKeepContents ) { // save the clipboard eventually
      QStringList dataList;
      if ( !bClipEmpty )
      {
          // don't iterate over the map, but over the popup (due to sorting!)
          long id = 0L;
          for ( uint i = 0; i < m_popup->count(); i++ ) {
              id = m_popup->idAt( i );
              if ( id != -1 ) {
                  QMapIterator<long,QString> it = m_clipDict.find( id );
                  if ( it != m_clipDict.end() )
                      dataList.append( it.data() );
              }
          }
      }

      KConfigGroupSaver groupSaver(m_config, "General");
      m_config->writeEntry("ClipboardData", dataList);
      m_config->sync();
  }
}


void TopLevel::slotConfigure()
{
    bool haveURLGrabber = bURLGrabber;
    if ( !myURLGrabber ) { // temporary, for the config-dialog
        setURLGrabberEnabled( true );
        readConfiguration( m_config );
    }

    ConfigDialog *dlg = new ConfigDialog( myURLGrabber->actionList(),
                                          globalKeys, isApplet() );
    dlg->setKeepContents( bKeepContents );
    dlg->setPopupAtMousePos( bPopupAtMouse );
    dlg->setReplayActionInHistory( bReplayActionInHistory );
    dlg->setNoNullClipboard( bNoNullClipboard );
    dlg->setUseGUIRegExpEditor( bUseGUIRegExpEditor );
    dlg->setPopupTimeout( myURLGrabber->popupTimeout() );
    dlg->setMaxItems( maxClipItems );
    dlg->setNoActionsFor( myURLGrabber->avoidWindows() );
//    dlg->setEnableActions( haveURLGrabber );

    if ( dlg->exec() == QDialog::Accepted ) {
        bKeepContents = dlg->keepContents();
        bPopupAtMouse = dlg->popupAtMousePos();
        bReplayActionInHistory = dlg->replayActionInHistory();
        bNoNullClipboard = dlg->noNullClipboard();
        bUseGUIRegExpEditor = dlg->useGUIRegExpEditor();
        dlg->commitShortcuts();
        globalKeys->writeSettings(m_config);
        globalKeys->updateConnections();
        toggleURLGrabAction->setShortcut(globalKeys->shortcut("Enable/Disable Clipboard Actions"));

//	haveURLGrabber = dlg->enableActions();

        myURLGrabber->setActionList( dlg->actionList() );
        myURLGrabber->setPopupTimeout( dlg->popupTimeout() );
	myURLGrabber->setAvoidWindows( dlg->noActionsFor() );

	maxClipItems = dlg->maxItems();
	trimClipHistory( maxClipItems );
	
        // KClipboard configuration
        m_config->setGroup("General");
        m_config->writeEntry("SynchronizeClipboardAndSelection", 
                             dlg->synchronize(), true, true );
        m_config->writeEntry("ImplicitlySetSelection", 
                             dlg->implicitSelection(), true, true );
        // ### notify running apps!
        // ------------------------

        writeConfiguration( m_config ); // syncs
    }
    setURLGrabberEnabled( haveURLGrabber );

    delete dlg;
}


void TopLevel::slotRepeatAction()
{
    if ( !myURLGrabber ) {
	myURLGrabber = new URLGrabber( m_config );
	connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
		 SLOT( showPopupMenu( QPopupMenu * )) );
    }

    myURLGrabber->invokeAction( m_lastString );
}

void TopLevel::setURLGrabberEnabled( bool enable )
{
    bURLGrabber = enable;
    toggleURLGrabAction->setChecked( enable );
    KConfig *kc = m_config;
    kc->setGroup("General");
    kc->writeEntry("URLGrabberEnabled", bURLGrabber);
    kc->sync();

    if ( !bURLGrabber ) {
        delete myURLGrabber;
        myURLGrabber = 0L;
        toggleURLGrabAction->setText(i18n("Enable &Actions"));
    }

    else {
        toggleURLGrabAction->setText(i18n("&Actions Enabled"));
        if ( !myURLGrabber ) {
            myURLGrabber = new URLGrabber( m_config );
            connect( myURLGrabber, SIGNAL( sigPopup( QPopupMenu * )),
                     SLOT( showPopupMenu( QPopupMenu * )) );
        }
    }
}

void TopLevel::toggleURLGrabber()
{
    setURLGrabberEnabled( !bURLGrabber );
}

void TopLevel::trimClipHistory( int new_size )
{
    while (m_popup->count() - MENU_ITEMS > (unsigned) new_size ) {
        int id = m_popup->idAt(EMPTY);
        if ( id == -1 )
            return;

        m_clipDict.remove(id);
        m_popup->removeItemAt(EMPTY);
    }
}

void TopLevel::removeFromHistory( const QString& text )
{
    QMapIterator<long,QString> it = m_clipDict.begin();
    for ( ; it != m_clipDict.end(); ++it ) {
        if ( it.data() == text ) {
            long id = it.key();
            m_popup->removeItem( id );
            m_clipDict.remove( id );
            return; // there can be only one (I hope :)
        }
    }
}

void TopLevel::slotClearClipboard()
{
    clip->setSelectionMode( true );
    clip->clear();
    clip->setSelectionMode( false );
    clip->clear();
    if ( m_selectedItem != -1 )
        m_popup->setItemEnabled(m_selectedItem, false);
}

QString TopLevel::clipboardContents( bool *isSelection )
{
    clip->setSelectionMode( true );
    QString clipContents = clip->text().stripWhiteSpace();
    if ( clipContents.isEmpty() ) {
        clip->setSelectionMode( false );
        clipContents = clip->text().stripWhiteSpace();
    }

    if ( isSelection )
        *isSelection = clip->selectionModeEnabled();

    return clipContents;
}

void TopLevel::applyClipChanges( const QString& clipData )
{
    m_lastString = clipData;

    if ( bURLGrabber && myURLGrabber ) {
        if ( myURLGrabber->checkNewData( clipData ))
            return; // don't add into the history
    }

    if (bClipEmpty) { // remove <clipboard empty> from popupmenu and dict
        if (clipData != QSempty) {
            bClipEmpty = false;
            m_popup->removeItemAt(EMPTY);
            m_clipDict.clear();
        }
    }

    if (m_selectedItem != -1)
        m_popup->setItemChecked(m_selectedItem, false);

    removeFromHistory( clipData );
    trimClipHistory(maxClipItems - 1);

    m_selectedItem = m_popup->insertItem(KStringHandler::csqueeze(clipData.simplifyWhiteSpace(), 45), -2, 1); // -2 means unique id, 1 means first location
    m_clipDict.insert(m_selectedItem, clipData);
    if ( bClipEmpty )
        m_popup->setItemEnabled( m_selectedItem, false );
    else
        m_popup->setItemChecked(m_selectedItem, true);
}

void TopLevel::setEmptyClipboard()
{
    bClipEmpty = true;
    applyClipChanges( QSempty );
}

void TopLevel::slotMoveSelectedToTop()
{
    m_popup->removeItem( m_selectedItem );
    m_clipDict.remove( m_selectedItem );

    m_selectedItem = m_popup->insertItem( KStringHandler::csqueeze(m_lastString.simplifyWhiteSpace(), 45), -2, 1 );
    m_popup->setItemChecked( m_selectedItem, true );
    m_clipDict.insert( m_selectedItem, m_lastString );
}

// clipboard polling for legacy apps
void TopLevel::newClipData()
{
    bool selectionMode;
    QString clipContents = clipboardContents( &selectionMode );
    checkClipData( clipContents, selectionMode );
}

void TopLevel::clipboardSignalArrived( bool selectionMode )
{
    clip->setSelectionMode( selectionMode );
    QString text = clip->text();

    checkClipData( text, selectionMode );
    m_checkTimer->start(1000);
}

void TopLevel::checkClipData( const QString& text, bool selectionMode )
{
    clip->setSelectionMode( selectionMode );

    bool clipEmpty = (clip->data()->format() == 0L);
    QString& lastClipRef = selectionMode ? m_lastSelection : m_lastClipboard;

    if ( text != lastClipRef ) {
        // keep old clipboard after someone set it to null
        if ( clipEmpty && bNoNullClipboard )
            setClipboard( lastClipRef, selectionMode );
        else
            lastClipRef = text;
    }

    // lastClipRef has the new value now, if any

    // If the string is null bug out
    if (lastClipRef.isEmpty()) {
	if (m_selectedItem != -1) {
            m_popup->setItemChecked(m_selectedItem, false);
	    m_selectedItem = -1;
	}

        if ( m_clipDict.isEmpty() ) {
            setEmptyClipboard();
        }
        return;
    }

    if (lastClipRef != m_lastString) {
        applyClipChanges( lastClipRef );
    }
}

void TopLevel::setClipboard( const QString& text, int mode )
{
    bool blocked = clip->signalsBlocked();
    clip->blockSignals( true ); // ### this might break other kicker applets

    if ( mode & Selection ) {
        clip->setSelectionMode( true );
        clip->setText( text );
    }
    if ( mode & Clipboard ) {
        clip->setSelectionMode( false );
        clip->setText( text );
    }

    clip->blockSignals( blocked );
}

#include "toplevel.moc"
