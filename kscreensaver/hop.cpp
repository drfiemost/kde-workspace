/*-
 * hop.c - Real Plane Fractals for xlock, the X Window System lockscreen.
 *
 * Copyright (c) 1991 by Patrick J. Naughton.
 *
 * See xlock.c for copying information.
 *
 * Revision History:
 * Changes of David Bagley <bagleyd@hertz.njit.edu>
 * 27-Jul-95: added Peter de Jong's hop from Scientific American
 *            July 87 p. 111.  Sometimes they are amazing but there are a
 *            few duds (I did not see a pattern in the parameters).
 * 29-Mar-95: changed name from hopalong to hop
 * 09-Dec-94: added sine hop
 *
 * Changes of Patrick J. Naughton
 * 29-Oct-90: fix bad (int) cast.
 * 29-Jul-90: support for multiple screens.
 * 08-Jul-90: new timing and colors and new algorithm for fractals.
 * 15-Dec-89: Fix for proper skipping of {White,Black}Pixel() in colors.
 * 08-Oct-89: Fixed long standing typo bug in RandomInitHop();
 *	      Fixed bug in memory allocation in init_hop();
 *	      Moved seconds() to an extern.
 *	      Got rid of the % mod since .mod is slow on a sparc.
 * 20-Sep-89: Lint.
 * 31-Aug-88: Forked from xlock.c for modularity.
 * 23-Mar-88: Coded HOPALONG routines from Scientific American Sept. 86 p. 14.
 */

/* Ported to kscreensave:
   July 1997, Emanuel Pirker <epirker@edu.uni-klu.ac.at>
   Last Revised: 10-Jul-97
   Contact me if something doesn't work correctly!
*/
// layout management added 1998/04/19 by Mario Weilguni <mweilguni@kde.org>

#include <qslider.h>
#include <kconfig.h>
#include "xlock.h"
#include <math.h>

#define MINSPEED 0
#define MAXSPEED 100
#define DEFSPEED 50
#define MINBATCH 0 
#define MAXBATCH 100
#define DEFBATCH 50
#define MINCYCLES 0
#define MAXCYCLES 1000
#define DEFCYCLES 500

#define SQRT 0
#define SIN 1
#define JONG 2
#define OPS 3

// ModeSpecOpt hop_opts = {0, NULL, NULL, NULL};

typedef struct {
	int         centerx;
	int         centery;	/* center of the screen */
	double      a;
	double      b;
	double      c;
	double      d;
	double      i;
	double      j;		/* hopalong parameters */
	int         inc;
	int         pix;
	int         op;
	int         count;
	int         bufsize;
} hopstruct;

static hopstruct hops[MAXSCREENS];
static XPoint pointBuffer[MAXBATCH];	/* pointer for XDrawPoints */

void
inithop(Window win)
{
  double      range;
  hopstruct  *hp = &hops[screen];
  XWindowAttributes xwa;

  (void) XGetWindowAttributes(dsp, win, &xwa);
  
  hp->centerx = xwa.width / 2;
  hp->centery = xwa.height / 2;
  /* Make the other operations less common since they are less interesting */
  if (LRAND() % 6)
    hp->op = SQRT;
  else
    hp->op = LRAND() % (OPS - 1) + 1;
  switch (hp->op) {
  case SQRT:
    range = sqrt((double) hp->centerx * hp->centerx +
		 (double) hp->centery * hp->centery) /
      (10.0 + LRAND() % 10);
    
    hp->a = (LRAND() / MAXRAND) * range - range / 2.0;
    hp->b = (LRAND() / MAXRAND) * range - range / 2.0;
    hp->c = (LRAND() / MAXRAND) * range - range / 2.0;
    if (LRAND() & 1)
      hp->c = 0.0;
    break;
  case SIN:
    hp->a = M_PI + ((LRAND() / MAXRAND) * 2.0 - 1.0) * 0.7;
    break;
  case JONG:
    hp->a = (LRAND() / MAXRAND) * 2.0 * M_PI - M_PI;
    hp->b = (LRAND() / MAXRAND) * 2.0 * M_PI - M_PI;
    hp->c = (LRAND() / MAXRAND) * 2.0 * M_PI - M_PI;
    hp->d = (LRAND() / MAXRAND) * 2.0 * M_PI - M_PI;
    break;
  }
  hp->pix = 0;
  hp->i = hp->j = 0.0;
  hp->inc = (int) ((LRAND() / MAXRAND) * 200) - 100;
  hp->bufsize = batchcount;
  
  //if (!pointBuffer)          // made a static structure, epirker, 10-Jul-97
  //  pointBuffer = (XPoint *) malloc(MAXBATCH * sizeof (XPoint));
  
  XSetForeground(dsp, Scr[screen].gc, BlackPixel(dsp, screen));
  XFillRectangle(dsp, win, Scr[screen].gc, 0, 0,
		 hp->centerx * 2, hp->centery * 2);
  XSetForeground(dsp, Scr[screen].gc, WhitePixel(dsp, screen));
  hp->count = 0;
}


void
drawhop(Window win)
{
	double      oldj, oldi;
	XPoint     *xp = pointBuffer;
	hopstruct  *hp = &hops[screen];
	int         k = hp->bufsize;

	hp->inc++;
	if (!mono && Scr[screen].npixels > 2) {
		XSetForeground(dsp, Scr[screen].gc, Scr[screen].pixels[hp->pix]);
		if (++hp->pix >= Scr[screen].npixels)
			hp->pix = 0;
	}
	while (k--) {
		oldj = hp->j;
		switch (hp->op) {
			case SQRT:
				oldi = hp->i + hp->inc;
				hp->j = hp->a - hp->i;
				hp->i = oldj + ((hp->i < 0)
					   ? sqrt(fabs(hp->b * oldi - hp->c))
					: -sqrt(fabs(hp->b * oldi - hp->c)));
				xp->x = hp->centerx + (int) (hp->i + hp->j);
				xp->y = hp->centery - (int) (hp->i - hp->j);
				break;
			case SIN:
				oldi = hp->i + hp->inc;
				hp->j = hp->a - hp->i;
				hp->i = oldj - sin(oldi);
				xp->x = hp->centerx + (int) (hp->i + hp->j);
				xp->y = hp->centery - (int) (hp->i - hp->j);
				break;
			case JONG:
				oldi = hp->i + 4 * hp->inc / hp->centerx;
				hp->j = sin(hp->c * hp->i) - cos(hp->d * hp->j);
				hp->i = sin(hp->a * oldj) - cos(hp->b * oldi);
				// included parenthesis after (int) to get rid of warnings
				// epirker, 10-Jul-97
				xp->x = hp->centerx + (int) (hp->centerx * (hp->i + hp->j)) / 4;
				xp->y = hp->centery - (int) (hp->centery * (hp->i - hp->j)) / 4;
				break;
		}
		xp++;
	}
	XDrawPoints(dsp, win, Scr[screen].gc,
		    pointBuffer, hp->bufsize, CoordModeOrigin);
	if (++hp->count > cycles )
		inithop(win);
}

//-----------------------------------------------------------------------------

#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qcolor.h>

#include "hop.h"

#include "hop.moc"

#include <qlayout.h>
#include <kbuttonbox.h>
#include "helpers.h"
#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>
#include <kmessagebox.h>

#undef Below // X sux

// this refers to klock.po. If you want an extra dictionary, 
// create an extra KLocale instance here.
//extern KLocale *glocale;

static kHopSaver *saver = NULL;

void startScreenSaver( Drawable d )
{
	if ( saver )
		return;
	saver = new kHopSaver( d );
}

void stopScreenSaver()
{
	if ( saver )
		delete saver;
	saver = NULL;
}

int setupScreenSaver()
{
	kHopSetup dlg;

	return dlg.exec();
}

//-----------------------------------------------------------------------------

kHopSaver::kHopSaver( Drawable drawable ) : kScreenSaver( drawable )
{
	readSettings();

	colorContext = QColor::enterAllocContext();

	batchcount = maxLevels;
	cycles = numPoints;

    // Clear to background colour when exposed
    XSetWindowBackground(qt_xdisplay(), mDrawable,
                            BlackPixel(qt_xdisplay(), qt_xscreen()));

	initXLock( mGc );
	inithop( mDrawable );

	timer.start( speed );
	connect( &timer, SIGNAL( timeout() ), SLOT( slotTimeout() ) );
}

kHopSaver::~kHopSaver()
{
	timer.stop();
	QColor::leaveAllocContext();
	QColor::destroyAllocContext( colorContext );
}

void kHopSaver::setSpeed( int spd )
{
	timer.stop();
	speed = 100-spd;
	timer.start( speed );
}

void kHopSaver::setLevels( int l )
{
	batchcount = maxLevels = l;
	inithop( mDrawable );
}

void kHopSaver::setPoints( int p )
{
	cycles = numPoints = p;
	inithop( mDrawable );
}

void kHopSaver::readSettings()
{
    KConfig *config = kapp->config();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = MAXSPEED - atoi( str );
	else
		speed = (MAXSPEED-MINSPEED)/2;

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = DEFBATCH;

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kHopSaver::slotTimeout()
{
	drawhop( mDrawable );
}

//-----------------------------------------------------------------------------

kHopSetup::kHopSetup( QWidget *parent, const char *name )
	: QDialog( parent, name, TRUE )
{

	readSettings();

	setCaption( i18n("Setup KHop") );

	QLabel *label;
	QPushButton *button;
	QSlider *slider;

	QVBoxLayout *tl = new QVBoxLayout(this, 10, 10);
	QHBoxLayout *tl1 = new QHBoxLayout;
	tl->addLayout(tl1);
	QVBoxLayout *tl11 = new QVBoxLayout(5);
	tl1->addLayout(tl11);		

	label = new QLabel( i18n("Speed:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new QSlider(MINSPEED, MAXSPEED, 10, speed, QSlider::Horizontal,
                        this);
	slider->setMinimumSize( 90, 20 );
    slider->setTickmarks(QSlider::Below);
    slider->setTickInterval(10);
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotSpeed( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(5); 

	label = new QLabel( i18n("Samecolor Pixels:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new QSlider(MINBATCH, MAXBATCH, 10, maxLevels,
                        QSlider::Horizontal, this);
	slider->setMinimumSize( 90, 20 );
    slider->setTickmarks(QSlider::Below);
    slider->setTickInterval(10);
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotLevels( int ) ) );
	tl11->addWidget(slider);
	tl11->addSpacing(5); 

	label = new QLabel( i18n("Cycles:"), this );
	min_size(label);
	tl11->addWidget(label);

	slider = new QSlider(MINCYCLES, MAXCYCLES, 100, numPoints,
                            QSlider::Horizontal, this);
	slider->setMinimumSize( 90, 20 );
    slider->setTickmarks(QSlider::Below);
    slider->setTickInterval(100);
	connect( slider, SIGNAL( valueChanged( int ) ), 
		 SLOT( slotPoints( int ) ) );
	tl11->addWidget(slider);
	tl11->addStretch(1);

	preview = new QWidget( this );
	preview->setFixedSize( 220, 170 );
	preview->setBackgroundColor( black );
	preview->show();    // otherwise saver does not get correct size
	saver = new kHopSaver( preview->winId() );
	tl1->addWidget(preview);

	KButtonBox *bbox = new KButtonBox(this);	
	button = bbox->addButton( i18n("About"));
	connect( button, SIGNAL( clicked() ), SLOT(slotAbout() ) );
	bbox->addStretch(1);

	button = bbox->addButton( i18n("OK"));	
	connect( button, SIGNAL( clicked() ), SLOT( slotOkPressed() ) );

	button = bbox->addButton(i18n("Cancel"));
	connect( button, SIGNAL( clicked() ), SLOT( reject() ) );
	bbox->layout();
	tl->addWidget(bbox);

	tl->freeze();	
}

void kHopSetup::readSettings()
{
	KConfig *config = KApplication::kApplication()->config();
	config->setGroup( "Settings" );

	QString str;

	str = config->readEntry( "Speed" );
	if ( !str.isNull() )
		speed = atoi( str );

	if ( speed > MAXSPEED )
		speed = MAXSPEED;
	else if ( speed < MINSPEED )
		speed = MINSPEED;

	str = config->readEntry( "MaxLevels" );
	if ( !str.isNull() )
		maxLevels = atoi( str );
	else
		maxLevels = DEFBATCH;

	str = config->readEntry( "NumPoints" );
	if ( !str.isNull() )
		numPoints = atoi( str );
	else
		numPoints = DEFCYCLES;
}

void kHopSetup::slotSpeed( int num )
{
	speed = num;

	if ( saver )
		saver->setSpeed( speed );
}

void kHopSetup::slotLevels( int num )
{
	maxLevels = num;

	if ( saver )
		saver->setLevels( maxLevels );
}

void kHopSetup::slotPoints( int num )
{
	numPoints = num;

	if ( saver )
		saver->setPoints( numPoints );
}

void kHopSetup::slotOkPressed()
{
	KConfig *config = KApplication::kApplication()->config();
	config->setGroup( "Settings" );

	QString sspeed;
	sspeed.setNum( speed );
	config->writeEntry( "Speed", sspeed );

	QString slevels;
	slevels.setNum( maxLevels );
	config->writeEntry( "MaxLevels", slevels );

	QString spoints;
	spoints.setNum( numPoints );
	config->writeEntry( "NumPoints", spoints );

	config->sync();
	accept();
}

void kHopSetup::slotAbout()
{
	KMessageBox::about(this,
			     i18n("Hop Version 3.3\n\nCopyright (c) 1991 by Patrick J. Naughton\n\nPorted to kscreensave by Emanuel Pirker."));
}


